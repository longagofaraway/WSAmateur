#include "serverGame.h"
#include "serverPlayer.h"
#include "serverProtocolHandler.h"

#include "lobbyEvent.pb.h"

ServerGame::ServerGame(size_t id, std::string description)
    : mId(id), mNextPlayerId(0), mDescription(description) {

}

void ServerGame::addPlayer(ServerProtocolHandler *client) {
    QMutexLocker locker(&gameMutex);
    auto player = std::make_shared<ServerPlayer>(this, client, mNextPlayerId++);
    mPlayers.emplace(player->id(), player);
    locker.unlock();

    EventGameJoined event;
    event.set_playerid(static_cast<google::protobuf::int32>(player->id()));
    client->sendLobbyEvent(event);
}
