#pragma once

#include <string>
#include <unordered_map>

#include <QMutex>

class ServerProtocolHandler;
class ServerPlayer;

class ServerGame
{
    size_t mId;
    size_t mNextPlayerId;
    std::string mDescription;
    QMutex gameMutex;
    std::unordered_map<size_t, std::shared_ptr<ServerPlayer>> mPlayers;

public:
    ServerGame(size_t id, std::string description);

    void addPlayer(ServerProtocolHandler *client);
};
