#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include <QMutex>

#include <google/protobuf/message.h>

#include "commands.h"
#include "globalAbilities/delayedAbility.h"
#include "serverPlayer.h"
#include "triggerManager.h"

class Server;
class ServerProtocolHandler;

class ServerGame
{
    Server *mServer;
    int mId;
    bool mFirstTurn = false;
    bool mGameEnded = false;
    int mNextPlayerId;
    std::string mDescription;
    std::unordered_map<int, std::unique_ptr<ServerPlayer>> mPlayers;
    asn::Phase mCurrentPhase = asn::Phase::Mulligan;

    std::optional<Resumable> mTask;

    TriggerManager mTriggerManager;
    std::vector<DelayedAbility> delayedAbilities;

public:
    ServerGame(Server *server, int id, std::string description);
    ~ServerGame();

    QMutex mGameMutex;

    int id() const { return mId; }
    const std::string& description() const { return mDescription; }

    void close();
    void sendPublicEvent(const ::google::protobuf::Message &event, int senderId);

    bool taskInProgress() const { return mTask.has_value(); }
    void startAsyncTask(Resumable&& task);
    void passCmdToTask(const GameCommand &cmd);
    void deleteTask() { mTask.reset(); }

    bool firstTurn() const { return mFirstTurn; }
    bool ended() const { return mGameEnded; }
    void setEnded() { mGameEnded = true; }
    std::unordered_map<int, std::unique_ptr<ServerPlayer>>& players() {
        return mPlayers;
    }
    ServerPlayer* player(int id);
    ServerPlayer* activePlayer(bool active = true);
    void addPlayer(ServerProtocolHandler *client);
    void removePlayer(int id);
    int playerCount() const { return static_cast<int>(mPlayers.size()); }
    ServerPlayer* opponentOfPlayer(int id) const;
    void setStartingPlayer();

    void sendGameInfo(ServerProtocolHandler *client, int recepientId);

    void startGame();
    Resumable endMulligan();
    asn::Phase phase() const { return mCurrentPhase; }
    void setPhase(asn::Phase phase) { mCurrentPhase = phase; }
    Resumable continueFromDamageStep();
    Resumable battleStep();
    Resumable encoreStep();
    Resumable endPhase();

    TriggerManager* triggerManager() { return &mTriggerManager; }
    void checkPhaseTrigger(asn::PhaseState state, asn::Phase phase);

    Resumable checkTiming();
    Resumable processRuleActions();

    void resolveAllContAbilities();
    void removePositionalContBuffsBySource(ServerCard *source);
    void addDelayedAbility(const asn::AutoAbility &ability, CardImprint &thisCard,
                           int duration, int abilityId);
    void removeDelayedAbility(const asn::AutoAbility &ability, CardImprint &thisCard, int abilityId);

private:
    void setTask(Resumable&& task) { mTask.emplace(std::move(task)); }

    void endOfTurnEffectValidation();
};
