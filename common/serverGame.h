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
    AttackDeclarationStep,
    DamageStep,
    BattleStep
};

class ServerGame
{
    int mId;
    int mNextPlayerId;
    std::string mDescription;
    std::unordered_map<int, std::unique_ptr<ServerPlayer>> mPlayers;
    Phase mCurrentPhase = Phase::Mulligan;

public:
    ServerGame(int id, std::string description);

    QMutex mGameMutex;

    std::unordered_map<int, std::unique_ptr<ServerPlayer>>& players() {
        return mPlayers;
    }
    ServerPlayer* player(int id);
    ServerPlayer* activePlayer();
    void addPlayer(ServerProtocolHandler *client);
    ServerPlayer* opponentOfPlayer(int id) const;
    void setStartingPlayer();
    void startGame();
    void endMulligan();
    Phase phase() const { return mCurrentPhase; }
    void setPhase(Phase phase) { mCurrentPhase = phase; }
    void battleStep();
    void encorePhase();

    void sendPublicEvent(const ::google::protobuf::Message &event, int senderId);
};
