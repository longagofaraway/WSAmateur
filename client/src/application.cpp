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
#include "remoteClientConnection.h"
#include "settingsManager.h"

#include <QDebug>

WSApplication::WSApplication() {
    qRegisterMetaType<std::shared_ptr<GameEvent>>("std::shared_ptr<GameEvent>");
    qRegisterMetaType<std::shared_ptr<SessionEvent>>("std::shared_ptr<SessionEvent>");
    qRegisterMetaType<std::shared_ptr<LobbyEvent>>("std::shared_ptr<LobbyEvent>");
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");

    try {
        // init db
        CardDatabase::get().init();
    } catch (const std::exception &) {
    }
}

WSApplication::~WSApplication() {
    clientThread.quit();
    clientThread.wait();
}

void WSApplication::componentComplete() {
    QQuickItem::componentComplete();

    initialization();
}

void WSApplication::initialization() {
    if (initPhase == InitPhase::ImageFileLinks) {
        if (!checkImageLinksFile())
            return;
        initPhase = InitPhase::Username;
    }

    if (initPhase == InitPhase::Username) {
        if (!checkUsername())
            return;
        initPhase = InitPhase::Done;
    }

    connectToServer();
}

void WSApplication::initGame(Game *game) {
    if (!connectionFailed)
        game->startNetworkGame(client.get(), playerId);
    else
        game->startLocalGame();
}

void WSApplication::initLobby(Lobby *lobby) {
    lobby->init(client.get());
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

bool WSApplication::checkImageLinksFile() {
    QFile imageLinksFile(paths::imageLinksPath());
    if (!imageLinksFile.exists()) {
        emit imageLinksFileNotFound();
        return false;
    }

    if (!ImageLinks::get().setData(paths::imageLinksPath())) {
        emit imageLinksFileNotFound();
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

void WSApplication::processSessionEvent(const std::shared_ptr<SessionEvent> event) {
    if (event->event().Is<EventServerHandshake>()) {
        EventServerHandshake ev;
        event->event().UnpackTo(&ev);
        processHandshake(ev);
    } else if (event->event().Is<EventDatabase>()) {
        EventDatabase ev;
        event->event().UnpackTo(&ev);
        updateDatabase(ev);
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

void WSApplication::userIdenditification() {
    CommandUserIdentification cmd;
    cmd.set_name(SettingsManager::get().getUsername().toStdString());
    client->sendSessionCommand(cmd);
}

void WSApplication::connectToServer() {
    auto conn = std::make_unique<RemoteClientConnection>();

    client = std::make_unique<Client>(std::move(conn));
    client->moveToThread(&clientThread);
    connect(client.get(), &Client::sessionEventReceived, this, &WSApplication::processSessionEvent);
    connect(client.get(), &Client::lobbyEventReceived, this, &WSApplication::processLobbyEvent);
    connect(client.get(), &Client::connectionClosed, this, &WSApplication::onConnectionClosed);

    clientThread.start();
    client->connectToHost("127.0.0.1", 7474);
}

void WSApplication::onConnectionClosed() {
    connectionFailed = true;
    //emit startGame();
}

namespace {
std::vector<int> parseVersion(const std::string &version) {
    std::stringstream versionStream(version);
    std::vector<int> res;

    std::string tmp;
    while(getline(versionStream, tmp, '.')){
        res.push_back(std::stoi(tmp));
    }

    if (res.size() != 3)
        throw std::runtime_error("wrong version format");

    return res;
}
}

void WSApplication::processHandshake(const EventServerHandshake &event) {
    try {
        client->setId(event.client_id());

        auto clientVersion = parseVersion(VERSION_STRING);
        auto serverVersion = parseVersion(event.version());

        // check major and minor
        if (clientVersion[1] != serverVersion[1] ||
            clientVersion[0] != serverVersion[0]) {
            emit needUpdate();
            return;
        }

        auto &cardDb = CardDatabase::get();
        if (!cardDb.initialized() || cardDb.version() != event.database_version()) {
            sendDatabaseRequest();
        } else {
            userIdenditification();
            enterLobby();
        }
    } catch (const std::exception &e) {
        qDebug() << e.what();
        // TODO do something
    }
}

void WSApplication::updateDatabase(const EventDatabase &event) {
    auto db = event.database();
    try {
        CardDatabase::get().update(db);
        CardDatabase::get().init();
    } catch (const std::exception &) {
        emit error();
    }
    userIdenditification();
    enterLobby();
}

void WSApplication::sendDatabaseRequest() {
    client->sendSessionCommand(CommandGetDatabase());
}

void WSApplication::enterLobby() {
    emit loadLobby();
}
