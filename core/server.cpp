#include "server.h"

#include <QTimer>

#include "commandContainer.pb.h"
#include "lobbyCommand.pb.h"
#include "serverMessage.pb.h"

#include "globalAbilities/globalAbilities.h"

#include <QDebug>

Server::Server(std::unique_ptr<ConnectionManager> cm)
    : mConnectionManager(std::move(cm)), mNextGameId(0) {
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");
    qRegisterMetaType<std::shared_ptr<ServerMessage>>("std::shared_ptr<ServerMessage>");

    decodeGlobalAbilities();

    mConnectionManager->initialize(this);

    if (!mConnectionManager->isLocal()) {
        mPingClock = new QTimer(this);
        connect(mPingClock, SIGNAL(timeout()), this, SIGNAL(pingClockTimeout()));
        mPingClock->start(mClientKeepalive  * 1000);
    }
}

EventGameList Server::gameList() {
    QReadLocker locker(&mGamesLock);
    EventGameList gamesInfo;
    for (const auto &game: mGames) {
        auto newGame = gamesInfo.add_games();
        newGame->set_id(game.second->id());
        newGame->set_name(game.second->description());
    }
    return gamesInfo;
}

int Server::nextGameId() {
    return ++mNextGameId;
}

ServerGame* Server::game(int id) {
    if (!mGames.count(id))
        return nullptr;

    return mGames.at(id).get();
}

ServerProtocolHandler* Server::addClient(std::unique_ptr<ServerProtocolHandler> client) {
    QWriteLocker locker(&mClientsLock);
    return mClients.emplace_back(std::move(client)).get();
}

void Server::removeClient(ServerProtocolHandler *client) {
    QWriteLocker locker(&mClientsLock);
    auto it = std::find_if(mClients.begin(), mClients.end(), [client](auto &elem) { return elem.get() == client; });
    if (it == mClients.end())
        return;
    it->release();
    mClients.erase(it);
}

void Server::createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client) {
    QWriteLocker locker(&mGamesLock);
    int newGameId = nextGameId();
    auto newGame = mGames.emplace(newGameId, std::make_unique<ServerGame>(newGameId, cmd.description())).first->second.get();
    locker.unlock();

    QReadLocker readLocker(&mGamesLock);
    newGame->addPlayer(client);
    qDebug() << "game created";
}

void Server::removeGame(int id) {
    QWriteLocker locker(&mGamesLock);

    if (!mGames.contains(id))
        return;

    auto &game = mGames.at(id);
    QMutexLocker gameLocker(&game->mGameMutex);
    game->close();
    gameLocker.unlock();
    mGames.erase(id);
}

void Server::processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client) {
    QReadLocker locker(&mGamesLock);
    auto g = game(cmd.game_id());
    if (!g)
        return;
    g->addPlayer(client);
}

int Server::maxClientInactivityTime() const {
    return mConnectionManager->isLocal() ? 999999 : 15;
}
