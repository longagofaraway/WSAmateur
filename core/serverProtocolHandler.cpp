#include "serverProtocolHandler.h"

#include <exception>

#include "commandContainer.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "serverMessage.pb.h"

#include "serverPlayer.h"
#include "server.h"
#include "serverGame.h"

#include <QDebug>

ServerProtocolHandler::ServerProtocolHandler(Server *server, std::unique_ptr<Connection> &&connection)
    : mServer(server), mGameId(0), mPlayerId(0) {
    connection->setParent(this);
    mConnection = connection.release();
    connect(mConnection, SIGNAL(messageReady(std::shared_ptr<CommandContainer>)),
            this, SLOT(processCommand(std::shared_ptr<CommandContainer>)));
    connect(mConnection, SIGNAL(connectionClosed()), this, SLOT(onConnectionClosed()));
    connect(this, SIGNAL(outputQueueChanged()), this, SLOT(flushOutputQueue()), Qt::QueuedConnection);
}

ServerProtocolHandler::~ServerProtocolHandler()
{
    flushOutputQueue();
}

void ServerProtocolHandler::initConnection() {
    mConnection->init();

    auto event = mServer->gameList();
    sendLobbyEvent(event);
}

void ServerProtocolHandler::flushOutputQueue() {
    QMutexLocker locker(&mOutputQueueMutex);
    if (mOutputQueue.empty() || !mConnection)
        return;

    while (!mOutputQueue.empty()) {
        auto message = mOutputQueue.front();
        mOutputQueue.pop();
        locker.unlock();

        mConnection->sendMessage(message);

        locker.relock();
    }
    mConnection->flush();
}

void ServerProtocolHandler::onConnectionClosed() {
    if (!mConnection)
        return;

    mServer->removeClient(this);

    QReadLocker locker(&mServer->mGamesLock);
    auto game = mServer->game(mGameId);
    if (game) {
        QMutexLocker gameLocker(&game->mGameMutex);
        game->removePlayer(mPlayerId);
        int playerCount = game->playerCount();
        if (playerCount == 0) {
            gameLocker.unlock();
            locker.unlock();
            mServer->removeGame(game->id());
        }
    }

    mConnection = nullptr;
    locker.unlock();
    deleteLater();
}

void ServerProtocolHandler::processCommand(std::shared_ptr<CommandContainer> cont) {
    try {
    if (cont->command().Is<LobbyCommand>()) {
        LobbyCommand lobbyCmd;
        cont->command().UnpackTo(&lobbyCmd);
        processLobbyCommand(lobbyCmd);
    } else if (cont->command().Is<GameCommand>()) {
        GameCommand gameCmd;
        cont->command().UnpackTo(&gameCmd);
        processGameCommand(gameCmd);
    }
    } catch (const std::exception &) {}
}


void ServerProtocolHandler::processLobbyCommand(LobbyCommand &cmd) {
    if (cmd.command().Is<CommandCreateGame>()) {
        CommandCreateGame createCmd;
        cmd.command().UnpackTo(&createCmd);
        mServer->createGame(createCmd, this);
    } else if (cmd.command().Is<CommandJoinGame>()) {
        CommandJoinGame joinCmd;
        cmd.command().UnpackTo(&joinCmd);
        mServer->processGameJoinRequest(joinCmd, this);
    }
}

void ServerProtocolHandler::processGameCommand(GameCommand &cmd) {
    QReadLocker locker(&mServer->mGamesLock);

    auto game = mServer->game(mGameId);
    if (!game)
        return;

    QMutexLocker gameLocker(&game->mGameMutex);

    auto player = game->player(mPlayerId);
    if (!player)
        return;

    player->processGameCommand(cmd);
}

void ServerProtocolHandler::sendProtocolItem(const ::google::protobuf::Message &event) {
    auto message = std::make_shared<ServerMessage>();
    message->mutable_message()->PackFrom(event);

    QMutexLocker locker(&mOutputQueueMutex);
    mOutputQueue.push(message);
    locker.unlock();

    // working through signals here to use only client's thread
    emit outputQueueChanged();
}

void ServerProtocolHandler::sendLobbyEvent(const ::google::protobuf::Message &event) {
    LobbyEvent lobbyEvent;
    lobbyEvent.mutable_event()->PackFrom(event);
    sendProtocolItem(lobbyEvent);
}

void ServerProtocolHandler::sendGameEvent(const ::google::protobuf::Message &event, int playerId) {
    GameEvent gameEvent;
    gameEvent.set_player_id(static_cast<::google::protobuf::uint32>(playerId));
    gameEvent.mutable_event()->PackFrom(event);
    sendProtocolItem(gameEvent);
}

void ServerProtocolHandler::addGameAndPlayer(int gameId, int playerId) {
    mGameId = gameId;
    mPlayerId = playerId;
}
