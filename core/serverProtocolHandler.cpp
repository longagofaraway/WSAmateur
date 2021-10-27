#include "serverProtocolHandler.h"

#include <exception>

#include "commandContainer.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "serverMessage.pb.h"
#include "sessionCommand.pb.h"
#include "sessionEvent.pb.h"

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

    connect(server, SIGNAL(pingClockTimeout()), this, SLOT(pingClockTimeout()));
}

ServerProtocolHandler::~ServerProtocolHandler() {
    flushOutputQueue();
}

void ServerProtocolHandler::initConnection() {
    try {
        mConnection->init();

        mServer->sendServerIdentification(this);
        auto event = mServer->gameList();
        sendLobbyEvent(event);
    } catch (const std::exception &e) {
        qDebug() << QString::fromStdString(e.what());
    }
}

void ServerProtocolHandler::flushOutputQueue() {
    try {
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
    } catch (const std::exception &e) {
        qDebug() << QString::fromStdString(e.what());
    }
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

void ServerProtocolHandler::pingClockTimeout() {
    if (timeRunning - lastDataReceived > mServer->maxClientInactivityTime())
        onConnectionClosed();
    timeRunning++;
}

void ServerProtocolHandler::processCommand(std::shared_ptr<CommandContainer> cont) {
    lastDataReceived = timeRunning;
    try {
    if (cont->command().Is<SessionCommand>()) {
        SessionCommand sessCmd;
        cont->command().UnpackTo(&sessCmd);
        processSessionCommand(sessCmd);
    } else if (cont->command().Is<LobbyCommand>()) {
        LobbyCommand lobbyCmd;
        cont->command().UnpackTo(&lobbyCmd);
        processLobbyCommand(lobbyCmd);
    } else if (cont->command().Is<GameCommand>()) {
        GameCommand gameCmd;
        cont->command().UnpackTo(&gameCmd);
        processGameCommand(gameCmd);
    }
    } catch (const std::exception &e) {
        qDebug() << QString::fromStdString(e.what());
    }
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
    } else if (cmd.command().Is<CommandSubscribeForGameList>()) {
        CommandSubscribeForGameList subscribeEvent;
        cmd.command().UnpackTo(&subscribeEvent);
        mServer->addGameListSubscriber(this);
    }
}

void ServerProtocolHandler::processGameCommand(GameCommand &cmd) {
    QReadLocker locker(&mServer->mGamesLock);

    auto game = mServer->game(mGameId);
    if (!game)
        return;

    QMutexLocker gameLocker(&game->mGameMutex);

    if (cmd.command().Is<CommandGetGameInfo>()) {
        game->sendGameInfo(this, mPlayerId);
        return;
    }

    auto player = game->player(mPlayerId);
    if (!player)
        return;

    player->processGameCommand(cmd);
}

void ServerProtocolHandler::processSessionCommand(const SessionCommand &cmd) {
    if (cmd.command().Is<CommandPing>()) {
        sendSessionEvent(EventPing());
    } else if (cmd.command().Is<CommandGetDatabase>()) {
        mServer->sendDatabase(this);
    }
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

void ServerProtocolHandler::sendSessionEvent(const ::google::protobuf::Message &event) {
    SessionEvent sessionEvent;
    sessionEvent.mutable_event()->PackFrom(event);
    sendProtocolItem(sessionEvent);
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
