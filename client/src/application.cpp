#include "application.h"

#include <sstream>

#include "lobbyEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "sessionEvent.pb.h"
#include "sessionCommand.pb.h"

#include <version_string.h>

#include "cardDatabase.h"
#include "game.h"
#include "remoteClientConnection.h"

#include <QDebug>

WSApplication::WSApplication() {
    qRegisterMetaType<std::shared_ptr<GameEvent>>("std::shared_ptr<GameEvent>");
    qRegisterMetaType<std::shared_ptr<SessionEvent>>("std::shared_ptr<SessionEvent>");
    qRegisterMetaType<std::shared_ptr<LobbyEvent>>("std::shared_ptr<LobbyEvent>");
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");

    try {
        // init db
        CardDatabase::get().init();
    } catch (const std::exception &e) {
    }
}

WSApplication::~WSApplication() {
    clientThread.quit();
    clientThread.wait();
}

void WSApplication::initGame(Game *game) {
    if (!connectionFailed)
        game->startNetworkGame(client.get(), playerId);
    else
        game->startLocalGame();
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
    } else if (event->event().Is<EventLobbyInfo>()) {
        EventLobbyInfo ev;
        event->event().UnpackTo(&ev);
        lobbyInfoReceived(ev);
    }
}

void WSApplication::lobbyInfoReceived(const EventLobbyInfo &event) {
    emit showGameList();
    return;
    // for testing purposes
    static bool gameStarted = false;
    if (gameStarted)
        return;
    gameStarted = true;

    if (!event.user_info_size()) {
        client->sendLobbyCommand(CommandEnterLobby());
        return;
    }

    CommandInviteToPlay cmd;
    cmd.set_user_id(event.user_info(0).id());
    client->sendLobbyCommand(cmd);
    /*if (!event->games_size()) {
        CommandCreateGame cmd;
        cmd.set_description("lol");
        client->sendLobbyCommand(cmd);
        return;
    }

    const auto &game = event->games(0);
    CommandJoinGame cmd;
    cmd.set_game_id(game.id());
    client->sendLobbyCommand(cmd);*/
}

void WSApplication::gameJoined(const EventGameJoined &event) {
    playerId = event.player_id();
    //emit startGame();
}

void WSApplication::onConnectionClosed() {
    connectionFailed = true;
    //emit startGame();
}

void WSApplication::componentComplete() {
    QQuickItem::componentComplete();

    auto conn = std::make_unique<RemoteClientConnection>();

    client = std::make_unique<Client>(std::move(conn));
    client->moveToThread(&clientThread);
    connect(client.get(), &Client::sessionEventReceived, this, &WSApplication::processSessionEvent);
    connect(client.get(), &Client::lobbyEventReceived, this, &WSApplication::processLobbyEvent);
    connect(client.get(), &Client::connectionClosed, this, &WSApplication::onConnectionClosed);

    clientThread.start();
    client->connectToHost("127.0.0.1", 7474);
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
    auto clientVersion = parseVersion(VERSION_STRING);
    auto serverVersion = parseVersion(event.version());

    // check major and minor
    if (clientVersion[1] != serverVersion[1] ||
        clientVersion[0] != serverVersion[0]) {
        emit needUpdate();
        return;
    }

    auto &cardDb = CardDatabase::get();
    if (!cardDb.initialized() || cardDb.version() != event.database_version())
        sendDatabaseRequest();
    else
        enterLobby();
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
    } catch (const std::exception &e) {
        emit error();
    }
    enterLobby();
}

void WSApplication::sendDatabaseRequest() {
    client->sendSessionCommand(CommandGetDatabase());
}

void WSApplication::enterLobby() {
    client->sendLobbyCommand(CommandEnterLobby());
}
