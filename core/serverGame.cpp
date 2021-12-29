#include "serverGame.h"

#include <random>

#include "serverProtocolHandler.h"

#include "gameEvent.pb.h"
#include "gameCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "phaseEvent.pb.h"

#include <QDebug>

ServerGame::ServerGame(int id, std::string description)
    : mId(id), mNextPlayerId(0), mDescription(description) {}

void ServerGame::close() {
    for (auto &playerEntry: mPlayers) {
        playerEntry.second->sendGameEvent(EventGameClosed());
    }
    mPlayers.clear();
}

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
    mPlayers[newId]->addExpectedCommand(CommandSetDeck::descriptor()->name());

    client->addGameAndPlayer(mId, newId);

    EventGameJoined event;
    event.set_player_id(static_cast<google::protobuf::uint32>(newId));
    event.set_game_id(static_cast<google::protobuf::uint32>(mId));
    client->sendLobbyEvent(event);

    // send event to the opponent
    for (const auto &[id, opponent]: mPlayers) {
        if (id == newId)
            continue;
        EventPlayerJoined evJoined;
        auto playerInfo = evJoined.mutable_player_info();
        playerInfo->set_id(newId);
        opponent->sendGameEvent(evJoined);
    }
}

void ServerGame::removePlayer(int id) {
    mPlayers.erase(id);

    if (mPlayers.empty())
        return;

    for (const auto &p: mPlayers) {
        p.second->sendGameEvent(EventPlayerLeft());
    }
}

ServerPlayer* ServerGame::opponentOfPlayer(int id) const {
    for (auto &playerEntry: mPlayers) {
        if (playerEntry.first != id)
            return playerEntry.second.get();
    }

    return nullptr;
}

void ServerGame::setStartingPlayer() {
    std::mt19937 gen(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<> distrib(1, 2);
    int startingPlayerId = 1;//distrib(gen);
    for (auto &playerEntry: mPlayers) {
        if (playerEntry.first == static_cast<int>(startingPlayerId))
            playerEntry.second->setActive(true);
    }
}

void ServerGame::sendGameInfo(ServerProtocolHandler *client, int recepientId) {
    EventGameInfo ev;
    auto gameInfo = ev.mutable_game_info();
    gameInfo->set_id(mId);
    gameInfo->set_name(mDescription);
    for (const auto &[id, playerIt]: mPlayers) {
        auto playerRecord = gameInfo->add_players();
        playerRecord->set_id(id);
        if (playerIt->deck())
            playerRecord->set_deck(playerIt->deck()->deck());
    }
    client->sendGameEvent(ev, recepientId);
}

void ServerGame::startGame() {
    for (auto &playerEntry: mPlayers) {
        if (!playerEntry.second->ready()
            || !playerEntry.second->deck())
            return;
    }

    setStartingPlayer();
    mFirstTurn = true;
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
    return nullptr;
}

Resumable ServerGame::continueFromDamageStep() {
    auto attPlayer = activePlayer();
    auto defPlayer = activePlayer(false);
    if (!attPlayer || !defPlayer)
        co_return;

    attPlayer->sendPhaseEvent(asn::Phase::DamageStep);
    defPlayer->sendPhaseEvent(asn::Phase::DamageStep);
    co_await defPlayer->damageStep();
    co_await battleStep();
    co_await attPlayer->endOfAttack();
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

    setPhase(asn::Phase::BattleStep);

    checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::BattleStep);
    co_await checkTiming();

    auto attCard = attPlayer->attackingCard();
    if (!attCard || attCard->zone()->name() != "stage")
        co_return;

    auto battleOpponent = attPlayer->oppositeCard(attCard);
    if (!battleOpponent)
        co_return;

    auto savedOppState = battleOpponent->state();
    if (attCard->power() > battleOpponent->power()) {
        if (!battleOpponent->cannotBecomeReversed())
            opponent->setCardState(battleOpponent, asn::State::Reversed);
    } else if (attCard->power() < battleOpponent->power()) {
        if (!attCard->cannotBecomeReversed())
            attPlayer->setCardState(attCard, asn::State::Reversed);
    } else {
        if (!attCard->cannotBecomeReversed())
            attPlayer->setCardState(attCard, asn::State::Reversed);
        if (!battleOpponent->cannotBecomeReversed())
            opponent->setCardState(battleOpponent, asn::State::Reversed);
    }

    if (attCard->state() == asn::State::Reversed) {
        attPlayer->checkOnReversed(attCard);
        opponent->checkOnBattleOpponentReversed(battleOpponent, attCard);
    }
    if (battleOpponent->state() == asn::State::Reversed &&
        battleOpponent->state() != savedOppState) {
        opponent->checkOnReversed(battleOpponent);
        attPlayer->checkOnBattleOpponentReversed(attCard, battleOpponent);
    }
    co_await checkTiming();
}

Resumable ServerGame::encoreStep() {
    auto turnPlayer = activePlayer();
    auto opponent = opponentOfPlayer(turnPlayer->id());
    if (!turnPlayer || !opponent)
        co_return;

    opponent->clearExpectedComands();

    checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::EncoreStep);
    co_await checkTiming();

    co_await turnPlayer->encoreStep();
    co_await opponent->encoreStep();

    co_await endPhase();
    turnPlayer->setActive(false);
    opponent->setActive(true);

    resolveAllContAbilities();

    mFirstTurn = false;
    co_await opponent->startTurn();
}

Resumable ServerGame::endPhase() {
    auto turnPlayer = activePlayer();
    auto opponent = activePlayer(false);
    if (!turnPlayer || !opponent)
        co_return;

    checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::EndPhase);
    co_await checkTiming();

    co_await turnPlayer->discardDownTo7();
    turnPlayer->clearClimaxZone();

    co_await checkTiming();

    turnPlayer->endOfTurnEffectValidation();
    opponent->endOfTurnEffectValidation();
    resolveAllContAbilities();
}

void ServerGame::checkPhaseTrigger(asn::PhaseState state, asn::Phase phase) {
    ServerPlayer *turnPlayer = activePlayer();
    ServerPlayer *opponent = activePlayer(false);
    if (!turnPlayer || !opponent)
        return;

    turnPlayer->checkPhaseTrigger(state, phase);
    opponent->checkPhaseTrigger(state, phase);
}

Resumable ServerGame::checkTiming() {
    ServerPlayer *turnPlayer = activePlayer();
    ServerPlayer *opponent = activePlayer(false);
    if (!turnPlayer || !opponent)
        co_return;

    do {
        co_await turnPlayer->checkTiming();
        co_await opponent->checkTiming();
    } while (turnPlayer->hasActivatedAbilities() || opponent->hasActivatedAbilities());
}

Resumable ServerGame::processRuleActions() {
    ServerPlayer *turnPlayer = activePlayer();
    ServerPlayer *opponent = activePlayer(false);
    if (!turnPlayer || !opponent)
        co_return;

    // TODO: process rule actions correctly
    do {
        co_await turnPlayer->processRuleActions();
        co_await opponent->processRuleActions();
    } while (turnPlayer->hasTriggeredRuleActions() || opponent->hasTriggeredRuleActions());
}

void ServerGame::resolveAllContAbilities() {
    ServerPlayer *turnPlayer = activePlayer();
    ServerPlayer *opponent = activePlayer(false);
    if (!turnPlayer || !opponent)
        return;

    turnPlayer->resolveAllContAbilities();
    opponent->resolveAllContAbilities();
}

void ServerGame::removePositionalContBuffsBySource(ServerCard *source) {
    ServerPlayer *turnPlayer = activePlayer();
    ServerPlayer *opponent = activePlayer(false);
    if (!turnPlayer || !opponent)
        return;

    turnPlayer->removePositionalContBuffsBySource(source);
    opponent->removePositionalContBuffsBySource(source);
}
