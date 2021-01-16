#include "serverGame.h"

#include <random>

#include "serverProtocolHandler.h"

#include "gameEvent.pb.h"
#include "gameCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "phaseEvent.pb.h"

ServerGame::ServerGame(int id, std::string description)
    : mId(id), mNextPlayerId(0), mDescription(description) {

}

void ServerGame::sendPublicEvent(const ::google::protobuf::Message &event, int senderId) {
    for (auto &playersEntry: mPlayers) {
        if (playersEntry.first == senderId)
            continue;

        playersEntry.second->sendGameEvent(event, senderId);
    }
}

ServerPlayer* ServerGame::player(int id) {
    if (!mPlayers.count(id))
        return nullptr;

    return mPlayers.at(id).get();
}

void ServerGame::addPlayer(ServerProtocolHandler *client) {
    QMutexLocker locker(&mGameMutex);
    int newId = ++mNextPlayerId;
    auto player = std::make_unique<ServerPlayer>(this, client, newId);
    mPlayers.emplace(newId, std::move(player));
    mPlayers[newId]->addExpectedCommand(CommandSetDeck::GetDescriptor()->name());

    client->addGameAndPlayer(mId, newId);

    EventGameJoined event;
    event.set_playerid(static_cast<google::protobuf::uint32>(newId));
    event.set_gameid(static_cast<google::protobuf::uint32>(mId));
    client->sendLobbyEvent(event);
}

ServerPlayer* ServerGame::opponentOfPlayer(int id) const {
    for (auto &playerEntry: mPlayers) {
        if (playerEntry.first != id)
            return playerEntry.second.get();
    }

    return nullptr;
}

void ServerGame::setStartingPlayer() {
    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> distrib(1, 2);
    int startingPlayerId = 1;//distrib(gen);
    for (auto &playerEntry: mPlayers) {
        if (playerEntry.first == static_cast<int>(startingPlayerId))
            playerEntry.second->setStartingPlayer();
    }
}

void ServerGame::startGame() {
    for (auto &playerEntry: mPlayers) {
        if (!playerEntry.second->ready()
            || !playerEntry.second->deck())
            return;
    }

    setStartingPlayer();
    for (auto &playerEntry: mPlayers) {
        playerEntry.second->setupZones();
        playerEntry.second->startGame();
        playerEntry.second->dealStartingHand();
    }
}

void ServerGame::endMulligan() {
    for (auto &pEntry: mPlayers) {
        if (!pEntry.second->mulliganFinished())
            return;
    }

    for (auto &pEntry: mPlayers) {
        if (pEntry.second->startingPlayer())
            pEntry.second->startTurn();
    }
}

