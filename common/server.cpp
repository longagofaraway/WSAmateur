#include "server.h"

#include "commandContainer.pb.h"
#include "lobbyCommand.pb.h"
#include "serverMessage.pb.h"

Server::Server() : mNextGameId(0)
{
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");
    qRegisterMetaType<std::shared_ptr<ServerMessage>>("std::shared_ptr<ServerMessage>");
}

int Server::nextGameId() {
    return ++mNextGameId;
}

ServerGame* Server::game(int id) {
    if (!mGames.count(id))
        return nullptr;

    return mGames.at(id).get();
}

void Server::createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client) {
    QWriteLocker locker(&mGamesLock);
    int newGameId = nextGameId();
    mGames.emplace(newGameId, std::make_unique<ServerGame>(newGameId, cmd.description()));
    mGames[newGameId]->addPlayer(client);
}

void Server::processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client) {
    QReadLocker locker(&mGamesLock);
    auto g = game(cmd.gameid());
    if (!g)
        return;
    g->addPlayer(client);
}
