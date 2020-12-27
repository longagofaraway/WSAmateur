#pragma once

#include <string>
#include <unordered_map>

#include <QMutex>

#include <google/protobuf/message.h>

#include "commands.h"
#include "serverPlayer.h"

class ServerProtocolHandler;

enum class Phase {
    Mulligan,
    Climax,
    Attack
};

class ServerGame
{
    size_t mId;
    size_t mNextPlayerId;
    std::string mDescription;
    std::unordered_map<size_t, std::unique_ptr<ServerPlayer>> mPlayers;
    Phase mCurrentPhase = Phase::Mulligan;

public:
    ServerGame(size_t id, std::string description);

    QMutex mGameMutex;

    std::unordered_map<size_t, std::unique_ptr<ServerPlayer>>& players() {
        return mPlayers;
    }
    ServerPlayer* player(size_t id);
    void addPlayer(ServerProtocolHandler *client);
    ServerPlayer* opponentOfPlayer(size_t id) const;
    void setStartingPlayer();
    void startGame();
    void endMulligan();
    Phase phase() const { return mCurrentPhase; }
    void setPhase(Phase phase) { mCurrentPhase = phase; }

    void sendPublicEvent(const ::google::protobuf::Message &event, size_t senderId);
};
