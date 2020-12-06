#include "serverGame.h"
#include "serverProtocolHandler.h"

#include "lobbyEvent.pb.h"

ServerGame::ServerGame(size_t id, std::string description)
    : mId(id), mNextPlayerId(0), mDescription(description) {

}

void ServerGame::sendPublicEvent(const ::google::protobuf::Message &event, size_t senderId) {
    for (auto &playersEntry: mPlayers) {
        if (playersEntry.first == senderId)
            continue;

        playersEntry.second->sendGameEvent(event);
    }
}

ServerPlayer* ServerGame::player(size_t id) {
    if (!mPlayers.count(id))
        return nullptr;

    return mPlayers.at(id).get();
}

void ServerGame::addPlayer(ServerProtocolHandler *client) {
    QMutexLocker locker(&mGameMutex);
    size_t newId = ++mNextPlayerId;
    auto player = std::make_unique<ServerPlayer>(this, client, newId);
    mPlayers.emplace(newId, std::move(player));

    client->addGameAndPlayer(mId, newId);

    EventGameJoined event;
    event.set_playerid(static_cast<google::protobuf::uint32>(newId));
    event.set_gameid(static_cast<google::protobuf::uint32>(mId));
    client->sendLobbyEvent(event);
}

void ServerGame::startGame() {
    for (auto &playerEntry: mPlayers) {
        if (!playerEntry.second->ready()
            || !playerEntry.second->deck())
            return;
    }

    for (auto &playerEntry: mPlayers) {
        playerEntry.second->setupZones();

        playerEntry.second->dealStartingHand();
    }
}
