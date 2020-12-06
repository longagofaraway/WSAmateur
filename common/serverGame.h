#pragma once

#include <string>
#include <unordered_map>

#include <QMutex>

#include <google/protobuf/message.h>

#include "serverPlayer.h"

class ServerProtocolHandler;

class ServerGame
{
    size_t mId;
    size_t mNextPlayerId;
    std::string mDescription;
    std::unordered_map<size_t, std::unique_ptr<ServerPlayer>> mPlayers;

public:
    ServerGame(size_t id, std::string description);

    QMutex mGameMutex;

    std::unordered_map<size_t, std::unique_ptr<ServerPlayer>>& players() {
        return mPlayers;
    }
    ServerPlayer* player(size_t id);
    void addPlayer(ServerProtocolHandler *client);
    void startGame();

    void sendPublicEvent(const ::google::protobuf::Message &event, size_t senderId);
};
