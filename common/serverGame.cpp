#include "serverGame.h"

#include <random>

#include "serverProtocolHandler.h"

#include "gameEvent.pb.h"
#include "gameCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "phaseEvent.pb.h"

ServerGame::ServerGame(int id, std::string description)
    : mId(id), mNextPlayerId(0), mDescription(description) {}

void ServerGame::startAsyncTask(Resumable &&task) {
    if (!task.resume())
        setTask(std::move(task));
}

void ServerGame::passCmdToTask(const GameCommand &cmd) {
    if (mTask->passCommand(cmd))
        mTask.reset();
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
            playerEntry.second->setActive(true);
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

Resumable ServerGame::endMulligan() {
    for (auto &pEntry: mPlayers) {
        if (!pEntry.second->mulliganFinished())
            co_return;
    }

    for (auto &pEntry: mPlayers) {
        if (pEntry.second->active())
            co_await pEntry.second->startTurn();
    }
}


ServerPlayer* ServerGame::activePlayer(bool active) {
    for (auto &pEntry: mPlayers) {
        if (pEntry.second->active() == active)
            return pEntry.second.get();
    }
    assert(false);
    return nullptr;
}

Resumable ServerGame::continueFromDamageStep() {
    auto attPlayer = activePlayer();
    auto defPlayer = activePlayer(false);
    attPlayer->sendPhaseEvent(asn::Phase::DamageStep);
    defPlayer->sendPhaseEvent(asn::Phase::DamageStep);
    co_await defPlayer->damageStep();
    co_await battleStep();
    attPlayer->endOfAttack();
    if (attPlayer->canAttack())
        attPlayer->attackDeclarationStep();
    else
        co_await encoreStep();
}

Resumable ServerGame::battleStep() {
    ServerPlayer *attPlayer = activePlayer();
    ServerPlayer *opponent = activePlayer(false);
    if (!attPlayer || !opponent)
        co_return;

    if (attPlayer->attackType() == AttackType::SideAttack
        || attPlayer->attackType() == AttackType::DirectAttack)
        co_return;

    mCurrentPhase = ServerPhase::BattleStep;
    //at the beginning of battle step, check timing

    auto attCard = attPlayer->attackingCard();
    if (!attCard || attCard->zone()->name() != "stage")
        co_return;

    auto battleOpponent = attPlayer->battleOpponent(attCard);
    if (!battleOpponent)
        co_return;


    auto oppState = battleOpponent->state();
    if (attCard->power() > battleOpponent->power()) {
        opponent->setCardState(battleOpponent, StateReversed);
    } else if (attCard->power() < battleOpponent->power()) {
        attPlayer->setCardState(attCard, StateReversed);
    } else {
        attPlayer->setCardState(attCard, StateReversed);
        opponent->setCardState(battleOpponent, StateReversed);
    }
    if (battleOpponent->state() == CardState::StateReversed &&
        battleOpponent->state() != oppState) {
        attPlayer->checkOnBattleOpponentReversed(attCard, battleOpponent);
    }
    co_await checkTiming();
}

Resumable ServerGame::encoreStep() {
    auto turnPlayer = activePlayer();
    auto opponent = opponentOfPlayer(turnPlayer->id());
    opponent->clearExpectedComands();

    co_await turnPlayer->encoreStep();
    co_await opponent->encoreStep();

    co_await turnPlayer->endPhase();
    co_await opponent->endPhase();
    turnPlayer->setActive(false);
    opponent->setActive(true);
    co_await opponent->startTurn();
}

Resumable ServerGame::checkTiming() {
    ServerPlayer *aplayer = activePlayer();
    ServerPlayer *opponent = activePlayer(false);
    if (!aplayer || !opponent)
        co_return;

    while (true) {
        co_await aplayer->checkTiming();
        co_await opponent->checkTiming();
        if (aplayer->hasActivatedAbilities() ||
            opponent->hasActivatedAbilities())
            continue;
        break;
    }
}
