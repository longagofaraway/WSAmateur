#include "application.h"

#include "lobbyEvent.pb.h"
#include "lobbyCommand.pb.h"

#include "cardDatabase.h"
#include "game.h"
#include "remoteClientConnection.h"

#include <QDebug>

WSApplication::WSApplication() {
    qRegisterMetaType<std::shared_ptr<EventGameJoined>>("std::shared_ptr<EventGameJoined>");
    qRegisterMetaType<std::shared_ptr<GameEvent>>("std::shared_ptr<GameEvent>");
    qRegisterMetaType<std::shared_ptr<EventGameList>>("std::shared_ptr<EventGameList>");
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");

    try {
        // init db
        CardDatabase::get();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        throw;
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

void WSApplication::gameListReceived(const std::shared_ptr<EventGameList> event) {
    if (!event->games_size()) {
        CommandCreateGame cmd;
        cmd.set_description("lol");
        client->sendLobbyCommand(cmd);
        return;
    }

    const auto &game = event->games(0);
    CommandJoinGame cmd;
    cmd.set_game_id(game.id());
    client->sendLobbyCommand(cmd);
}

void WSApplication::gameJoined(const std::shared_ptr<EventGameJoined> event) {
    playerId = event->player_id();
    emit startGame();
}

void WSApplication::onConnectionClosed() {
    connectionFailed = true;
    emit startGame();
}

void WSApplication::componentComplete() {
    QQuickItem::componentComplete();

    auto conn = std::make_unique<RemoteClientConnection>();

    client = std::make_unique<Client>(std::move(conn));
    client->moveToThread(&clientThread);
    connect(client.get(), &Client::gameJoinedEventReceived, this, &WSApplication::gameJoined);
    connect(client.get(), &Client::gameListReceived, this, &WSApplication::gameListReceived);
    connect(client.get(), &Client::connectionClosed, this, &WSApplication::onConnectionClosed);

    clientThread.start();
    client->connectToHost("127.0.0.1", 7474);
}
