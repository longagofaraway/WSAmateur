#include "server.h"

#include "commandContainer.pb.h"
#include "lobbyCommand.pb.h"
#include "serverMessage.pb.h"

#include "serverGame.h"

Server::Server() : mNextGameId(0)
{
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");
    qRegisterMetaType<std::shared_ptr<ServerMessage>>("std::shared_ptr<ServerMessage>");
}

void Server::createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client) {
    size_t newGameId = nextGameId();
    auto game = std::make_shared<ServerGame>(newGameId, cmd.description());
    game->addPlayer(client);
    QWriteLocker locker(&mGamesLock);
    mGames.emplace(newGameId, game);
}

size_t Server::nextGameId() {
    QMutexLocker locker(&mNextGameIdMutex);
    return ++mNextGameId;
}
