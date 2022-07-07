#include "application.h"

#include <sstream>

#include "lobbyEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "sessionEvent.pb.h"
#include "sessionCommand.pb.h"

#include <version_string.h>

#include "cardDatabase.h"
#include "filesystemPaths.h"
#include "game.h"
#include "imageLinks.h"
#include "lobby.h"
#include "publicServers.h"
#include "remoteClientConnection.h"
#include "settingsManager.h"
#include "updater.h"
#include "versionParser.h"

#include <QDebug>

WSApplication::WSApplication() {
    qRegisterMetaType<std::shared_ptr<GameEvent>>("std::shared_ptr<GameEvent>");
    qRegisterMetaType<std::shared_ptr<SessionEvent>>("std::shared_ptr<SessionEvent>");
    qRegisterMetaType<std::shared_ptr<LobbyEvent>>("std::shared_ptr<LobbyEvent>");
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");

    publicServers = new PublicServers();
    connect(publicServers, &PublicServers::serversReady, this, &WSApplication::connectToHost);

    paths::setUpRootDirectory();
    try {
        // init db
        CardDatabase::get().init();
    } catch (const std::exception &) {
    }
}

WSApplication::~WSApplication() {
    clientThread.quit();
    clientThread.wait();
    if (client)
        client->deleteLater();
}

void WSApplication::startInitialization() {
    // invoke here because we need to let StartWindow to load
    QMetaObject::invokeMethod(this, "initialization", Qt::QueuedConnection);
}

void WSApplication::initialization() {
    if (!checkUsername())
        return;
    checkImageLinksFile();

    connectToServer();
}

void WSApplication::connectToHost(QString address, QString port) {
    auto &settingsManager = SettingsManager::get();
    if (settingsManager.localGameEnabled())
        client->connectToHost("127.0.0.1", 7474);
    else
        client->connectToHost(address, port.toInt());
}

void WSApplication::initGame(Game *game) {
    if (!connectionFailed)
        game->startNetworkGame(client, playerId);
    else
        game->startLocalGame();
}

void WSApplication::initLobby(Lobby *lobby) {
    lobby->init(client);
}

void WSApplication::imageLinksFileChosen(QString path) {
    if (!ImageLinks::get().setData(path)) {
        emit imageFileParseError();
        return;
    }

    emit imageFileParsed();

    initialization();
}

void WSApplication::setUsername(QString name) {
    SettingsManager::get().setUsername(name);
    emit usernameSet();

    initialization();
}

void WSApplication::gameEnded() {
    emit loadLobby();
}

bool WSApplication::checkImageLinksFile() {
    QFile imageLinksFile(paths::imageLinksPath());
    if (!imageLinksFile.exists()) {
        return false;
    }

    if (!ImageLinks::get().loadFile(paths::imageLinksPath())) {
        return false;
    }
    return true;
}

bool WSApplication::checkUsername() {
    auto &settingsManager = SettingsManager::get();
    if (!settingsManager.hasUsername()) {
        emit usernameNotFound();
        return false;
    }
    return true;
}

bool WSApplication::clientIsUpToDate() const {
    return dbIsUpToDate && imageLinksAreUpToDate;
}

void WSApplication::processSessionEvent(const std::shared_ptr<SessionEvent> event) {
    if (event->event().Is<EventServerHandshake>()) {
        EventServerHandshake ev;
        event->event().UnpackTo(&ev);
        processHandshake(ev);
    } else if (event->event().Is<EventDatabase>()) {
        EventDatabase ev;
        event->event().UnpackTo(&ev);
        updateDatabase(ev);
    } else if (event->event().Is<EventImageLinks>()) {
        EventImageLinks ev;
        event->event().UnpackTo(&ev);
        updateImageLinks(ev);
    }
}

void WSApplication::processLobbyEvent(const std::shared_ptr<LobbyEvent> event) {
    if (event->event().Is<EventGameJoined>()) {
        EventGameJoined ev;
        event->event().UnpackTo(&ev);
        gameJoined(ev);
    }
}

void WSApplication::gameJoined(const EventGameJoined &event) {
    playerId = event.player_id();
    emit startGame();
}

void WSApplication::userIdentification() {
    CommandUserIdentification cmd;
    cmd.set_name(SettingsManager::get().getUsername().toStdString());
    client->sendSessionCommand(cmd);
}

void WSApplication::connectToServer() {
    if (client)
        client->deleteLater();
    client = new Client(new RemoteClientConnection());
    client->moveToThread(&clientThread);
    connect(client, &Client::sessionEventReceived, this, &WSApplication::processSessionEvent);
    connect(client, &Client::lobbyEventReceived, this, &WSApplication::processLobbyEvent);
    connect(client, &Client::connectionClosed, this, &WSApplication::onConnectionClosed);

    if (!clientThread.isRunning())
        clientThread.start();
    publicServers->getServers();
}

void WSApplication::onConnectionClosed() {
    connectionFailed = true;
    client->deleteLater();
    client = nullptr;
    auto &settingsManager = SettingsManager::get();
    if (settingsManager.localGameEnabled())
        emit startGame();
    else
        emit error("Connection error");
}

void WSApplication::processHandshake(const EventServerHandshake &event) {
    try {
        client->setId(event.client_id());

        auto clientVersion = parseVersion(std::string(VERSION_STRING));
        auto serverVersion = parseVersion(event.version());

        // check major and minor
        if (clientVersion[1] != serverVersion[1] ||
            clientVersion[0] != serverVersion[0]) {
            startUpdater(event.version());
            return;
        }

        auto &cardDb = CardDatabase::get();
        if (!cardDb.initialized() || cardDb.version() != event.database_version()) {
            dbIsUpToDate = false;
            sendDatabaseRequest();
        }
        if (ImageLinks::get().version() != event.image_links_version()) {
            imageLinksAreUpToDate = false;
            sendImageLinksRequest();
        }
        tryEnterLobby();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        emit error(QString::fromStdString(e.what()));
    }
}

void WSApplication::updateDatabase(const EventDatabase &event) {
    auto db = event.database();
    try {
        CardDatabase::get().update(db);
        CardDatabase::get().init();
    } catch (const std::exception &) {
        emit error("Database update error");
        return;
    }
    dbIsUpToDate = true;
    tryEnterLobby();
}

void WSApplication::updateImageLinks(const EventImageLinks &event) {
    auto image_links = event.image_links();
    if (!ImageLinks::get().update(image_links)) {
        emit error("File with image links cannot be updated");
        return;
    }
    imageLinksAreUpToDate = true;
    tryEnterLobby();
}

void WSApplication::sendDatabaseRequest() {
    client->sendSessionCommand(CommandGetDatabase());
}

void WSApplication::sendImageLinksRequest() {
    client->sendSessionCommand(CommandGetImageLinks());
}

void WSApplication::tryEnterLobby() {
    if (!clientIsUpToDate())
        return;
    userIdentification();
    emit loadLobby();
}

void WSApplication::startUpdater(const std::string& neededVersion) {
    updater = new Updater(neededVersion);

    connect(this, &WSApplication::startUpdate, updater, &Updater::startUpdate);
    connect(updater, &Updater::error, this, &WSApplication::error);
    connect(updater, &Updater::progressMade, this, &WSApplication::progressMade);
    updater->moveToThread(&clientThread);

    emit needUpdate();
}
