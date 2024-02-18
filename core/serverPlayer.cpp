#include "serverPlayer.h"

#include <algorithm>

#include "ability.pb.h"
#include "abilityCommands.pb.h"
#include "abilityEvents.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "cardDatabase.h"
#include "serverCardZone.h"
#include "serverGame.h"
#include "serverProtocolHandler.h"
#include "serverStage.h"

#include <QDebug>

ServerPlayer::ServerPlayer(ServerGame *game, ServerProtocolHandler *client, int id)
    : mGame(game), mClient(client), mId(id), mBuffManager(this) {}

ServerPlayer::~ServerPlayer() {}

void ServerPlayer::disconnectClient() {
    QMutexLocker locker(&mPlayerMutex);
    mClient = nullptr;
}

void ServerPlayer::processGameCommand(GameCommand &cmd) {
    if (!expectsCommand(cmd))
        return;

    if (mGame->taskInProgress()) {
        mGame->passCmdToTask(cmd);
        return;
    }

    if (cmd.command().Is<CommandSetDeck>()) {
        CommandSetDeck setDeckCmd;
        cmd.command().UnpackTo(&setDeckCmd);
        addDeck(setDeckCmd.deck());
    } else if (cmd.command().Is<CommandReadyToStart>()) {
        CommandReadyToStart readyCmd;
        cmd.command().UnpackTo(&readyCmd);
        setReady(readyCmd.ready());
        mGame->startGame();
    } else if (cmd.command().Is<CommandMulligan>()) {
        CommandMulligan mulliganCmd;
        cmd.command().UnpackTo(&mulliganCmd);
        mGame->startAsyncTask(mulligan(mulliganCmd));
    } else if (cmd.command().Is<CommandClockPhase>()) {
        CommandClockPhase clockCmd;
        cmd.command().UnpackTo(&clockCmd);
        mGame->startAsyncTask(processClockPhaseResult(clockCmd));
    } else if (cmd.command().Is<CommandPlayCard>()) {
        CommandPlayCard playCmd;
        cmd.command().UnpackTo(&playCmd);
        mGame->startAsyncTask(playCard(playCmd));
    } else if (cmd.command().Is<CommandPlayCounter>()) {
        CommandPlayCounter counterCmd;
        cmd.command().UnpackTo(&counterCmd);
        mGame->startAsyncTask(playCounter(counterCmd));
    } else if (cmd.command().Is<CommandSwitchStagePositions>()) {
        CommandSwitchStagePositions switchCmd;
        cmd.command().UnpackTo(&switchCmd);
        mGame->startAsyncTask(switchPositions(switchCmd));
    } else if (cmd.command().Is<CommandClimaxPhase>()) {
        mGame->startAsyncTask(climaxPhase());
    } else if (cmd.command().Is<CommandAttackPhase>()) {
        mGame->startAsyncTask(startAttackPhase());
    } else if (cmd.command().Is<CommandDeclareAttack>()) {
        CommandDeclareAttack declareAttackCmd;
        cmd.command().UnpackTo(&declareAttackCmd);
        mGame->startAsyncTask(declareAttack(declareAttackCmd));
    } else if (cmd.command().Is<CommandTakeDamage>()) {
        mGame->startAsyncTask(mGame->continueFromDamageStep());
    } else if (cmd.command().Is<CommandEncoreStep>()) {
        mGame->startAsyncTask(mGame->encoreStep());
    } else if (cmd.command().Is<CommandPlayAct>()) {
        CommandPlayAct actCmd;
        cmd.command().UnpackTo(&actCmd);
        mGame->startAsyncTask(processPlayActCmd(actCmd));
    } else if (cmd.command().Is<CommandTest>()) {
        mGame->startAsyncTask(testAction());
    }

    if (mGame->ended()) {
        clearExpectedComands();
        mGame->opponentOfPlayer(mId)->clearExpectedComands();
        if (mGame->taskInProgress())
            mGame->deleteTask();
    }
}

void ServerPlayer::sendGameEvent(const ::google::protobuf::Message &event, int playerId) {
    if (!playerId)
        playerId = mId;

    QMutexLocker locker(&mPlayerMutex);
    if (mClient)
        mClient->sendGameEvent(event, playerId);
}

void ServerPlayer::sendToBoth(const google::protobuf::Message &event) {
    sendGameEvent(event);
    mGame->sendPublicEvent(event, mId);
}

void ServerPlayer::addDeck(const std::string &deck) {
    mDeck = std::make_unique<DeckList>();
    if (!mDeck->fromXml(deck)) {
        mDeck.reset();
        return;
    }

    mExpectedCommands.push_back(CommandReadyToStart::descriptor()->name());

    EventDeckSet event;
    event.set_player_id(mId);
    event.set_deck(deck);
    mGame->sendPublicEvent(event, mId);
}

ServerCardZone* ServerPlayer::addZone(std::string_view name, ZoneType type) {
    return mZones.emplace(name, std::make_unique<ServerCardZone>(this, name, type)).first->second.get();
}

ServerCardZone* ServerPlayer::zone(std::string_view name) {
    if (!mZones.count(name))
        return nullptr;

    return mZones.at(name).get();
}

ServerCardZone *ServerPlayer::zone(asn::Zone name) {
    return zone(asnZoneToString(name));
}

void ServerPlayer::setupZones() {
    auto deck = addZone("deck", ZoneType::HiddenZone);
    addZone("wr");
    addZone("hand", ZoneType::PrivateZone);
    addZone("clock");
    addZone("stock", ZoneType::HiddenZone);
    addZone("memory");
    addZone("climax");
    addZone("level");
    addZone("res");
    addZone("memory");
    createStage();

    int uniqueId = 1;
    for (auto &card: mDeck->cards()) {
        auto cardInfo = CardDatabase::get().getCard(card.code);
        for (int i = 0; i < card.count; ++i)
            deck->addCard(cardInfo, uniqueId++);
    }

    deck->shuffle();
}

void ServerPlayer::createStage() {
    mZones.emplace("stage", std::make_unique<ServerStage>(this));
}

void ServerPlayer::setReady(bool ready) {
    mReady = ready;
    EventPlayerReady event;
    event.set_player_id(mId);
    sendToBoth(event);
}

void ServerPlayer::startGame() {
    mExpectedCommands.clear();
    mExpectedCommands.emplace_back(CommandMulligan::descriptor()->name(), 1);
}

Resumable ServerPlayer::startTurn() {
    sendToBoth(EventStartTurn());

    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::StandPhase);
    co_await mGame->checkTiming();

    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (card && card->state() != asn::State::Standing && !card->cannotStand())
            setCardState(card, asn::State::Standing);
        if (card)
            card->buffManager()->validateCannotStand();
    }

    co_await mGame->checkTiming();

    mGame->setPhase(asn::Phase::DrawPhase);
    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::DrawPhase);
    co_await mGame->checkTiming();

    co_await drawCards(1);
    co_await mGame->checkTiming();

    mGame->setPhase(asn::Phase::ClockPhase);
    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::ClockPhase);
    co_await mGame->checkTiming();

    sendToBoth(EventClockPhase());

    addExpectedCommand(CommandClockPhase::descriptor()->name());
}

void ServerPlayer::addExpectedCommand(const std::string &command, int maxCount) {
    mExpectedCommands.emplace_back(command, maxCount);
}

void ServerPlayer::clearExpectedComands() {
    mExpectedCommands.clear();
}

bool ServerPlayer::expectsCommand(const GameCommand &command) {
    auto &fullCmdName = command.command().type_url();
    size_t pos = fullCmdName.find('/');
    if (pos == std::string::npos)
        throw std::runtime_error("error parsing GameCommand type");

    auto cmdName = fullCmdName.substr(pos + 1);

    auto it = std::find(mExpectedCommands.begin(), mExpectedCommands.end(), cmdName);
    if (it == mExpectedCommands.end())
        return false;

    if (it->commandArrived())
        mExpectedCommands.erase(it);

    return true;
}

void ServerPlayer::dealStartingHand() {
    auto deck = zone("deck");
    auto hand = zone("hand");

    EventInitialHand eventPrivate;
    eventPrivate.set_count(5);
    eventPrivate.set_first_turn(mActive);
    EventInitialHand eventPublic(eventPrivate);
    for (int i = 0; i < 5; ++i) {
        auto card = deck->takeTopCard();
        if (!card)
            break;
        auto code = eventPrivate.add_cards();
        code->set_code(card->code());
        code->set_id(card->id());
        hand->addCard(std::move(card));
    }

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);
}

Resumable ServerPlayer::mulligan(const CommandMulligan cmd) {
    if (cmd.ids_size()) {
        std::vector<int> ids;
        for (int i = 0; i < cmd.ids_size(); ++i)
            ids.push_back(cmd.ids(i));

        moveCards("hand", ids, "wr");
        co_await drawCards(static_cast<int>(ids.size()));
    }
    mMulliganFinished = true;
    co_await mGame->endMulligan();
}

Resumable ServerPlayer::drawCards(int number) {
    for (int i = 0; i < number; ++i)
        co_await moveTopDeck("hand");
}

void ServerPlayer::moveCards(std::string_view startZoneName, const std::vector<int> &cardPositions, std::string_view targetZoneName) {
    auto sortedPositions = cardPositions;
    std::sort(sortedPositions.begin(), sortedPositions.end());
    for (int i = static_cast<int>(sortedPositions.size()); i-- > 0;)
        moveCard(startZoneName, sortedPositions[i], targetZoneName);
}

bool ServerPlayer::moveCard(std::string_view startZoneName, int startPos, std::string_view targetZoneName,
                            int targetPos, bool reveal, bool enableGlobEncore) {
    ServerCardZone *startZone = zone(startZoneName);
    if (!startZone)
        return false;

    ServerCardZone *targetZone = zone(targetZoneName);
    if (!targetZone)
        return false;

    if (startZoneName != "stage" && targetZoneName == "stage")
        return moveCardToStage(startZone, startPos, targetZone, targetPos);
    if (startZoneName == "stage" && targetZoneName == "stage") {
        switchPositions(startPos, targetPos);
        return true;
    }

    ServerCard *card = startZone->card(startPos);
    if (!card)
        return false;

    // check trigger while temporary abilities like 'encore' are still present
    checkZoneChangeTrigger(card, startZoneName, targetZoneName);

    if (startZoneName == "stage" || startZoneName == "climax" || startZoneName == "hand") {
        // revert effects of cont abilities
        playContAbilities(card, true/*revert*/);
    }
    auto markers = std::move(card->markers());
    card->reset();
    if (card == attackingCard())
        setAttackingCard(nullptr);

    auto cardPtr = startZone->takeCard(startPos);

    targetZone->addCard(std::move(cardPtr), targetPos);

    EventMoveCard eventPublic;
    eventPublic.set_start_zone(startZone->name());
    eventPublic.set_target_zone(targetZone->name());
    eventPublic.set_start_pos(startPos);
    eventPublic.set_target_pos(targetPos);
    if (startZoneName == "stage" && markers.size()) {
        auto movedMarkers = moveMarkersToWr(markers);
        *eventPublic.mutable_markers() = { movedMarkers.begin(), movedMarkers.end() };
    }

    if (startZone->type() == ZoneType::PublicZone || targetZone->type() == ZoneType::PublicZone) {
        eventPublic.set_code(card->code());
        eventPublic.set_card_id(card->id());
    }

    EventMoveCard eventPrivate(eventPublic);

    if (startZone->type() == ZoneType::PrivateZone || targetZone->type() == ZoneType::PrivateZone) {
        eventPrivate.set_code(card->code());
        eventPrivate.set_card_id(card->id());
        if (reveal)
            eventPublic.set_code(card->code());
    }

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    mGame->resolveAllContAbilities();

    if (enableGlobEncore)
        checkGlobalEncore(card, startZoneName, targetZoneName);

    return true;
}


bool ServerPlayer::moveCardToStage(ServerCardZone *startZone, int startPos, ServerCardZone *stage, int targetPos) {
    if (targetPos >= 5)
        return false;

    auto card = startZone->takeCard(startPos);
    if (!card)
        return false;

    return moveCardToStage(std::move(card), startZone->name(), startPos, stage, targetPos);
}

bool ServerPlayer::moveCardToStage(std::unique_ptr<ServerCard> card, const std::string &startZoneName,
                                   int startPos, ServerCardZone *stage, int targetPos,
                                   std::optional<int> markerPos) {
    card->reset();

    // first, the card is placed on stage,
    // then if another card is already present on stage in the same place, it is removed by Rule Action
    // so old stage card can still trigger
    checkZoneChangeTrigger(card.get(), startZoneName, "stage");

    std::vector<std::unique_ptr<ServerCard>> removedMarkers;
    auto currentStageCard = stage->card(targetPos);
    if (currentStageCard) {
        // check trigger while temporary abilities like 'encore' are still present
        checkZoneChangeTrigger(currentStageCard, "stage", "wr");
        // revert effects of cont abilities
        playContAbilities(currentStageCard, true);
        removedMarkers = std::move(card->markers());
        currentStageCard->reset();
    }

    auto oldStageCard = stage->putOnStage(std::move(card), targetPos);
    auto cardOnStage = stage->card(targetPos);
    if (startZoneName == "hand")
        cardOnStage->setFirstTurn(true);

    moveMarkersToWr(removedMarkers);

    EventMoveCard event;
    event.set_card_id(cardOnStage->id());
    event.set_code(cardOnStage->code());
    event.set_start_zone(startZoneName);
    event.set_start_pos(startPos);
    event.set_target_zone(stage->name());
    event.set_target_pos(targetPos);
    if (markerPos.has_value()) {
        event.set_marker_pos(markerPos.value());
    }
    sendToBoth(event);

    if (oldStageCard)
        zone("wr")->addCard(std::move(oldStageCard));

    mGame->resolveAllContAbilities();

    return true;
}

Resumable ServerPlayer::moveTopDeck(std::string_view targetZoneName) {
    auto deck = zone("deck");
    moveCard("deck", deck->count() - 1, targetZoneName);

    co_await checkRefreshAndLevelUp();
}

std::vector<ProtoTypeCard> ServerPlayer::moveMarkersToWr(std::vector<std::unique_ptr<ServerCard>> &markers) {
    auto wr = zone("wr");
    std::vector<ProtoTypeCard> movedMarkers;
    for (auto &&marker: markers) {
        ProtoTypeCard movedMarker;
        movedMarker.set_code(marker->code());
        movedMarker.set_id(marker->id());
        movedMarkers.push_back(movedMarker);

        wr->addCard(std::move(marker));
    }
    markers.clear();
    return movedMarkers;
}

void ServerPlayer::addMarker(ServerCardZone *startZone, int startPos,
                             int targetPos, asn::FaceOrientation faceOrientation,
                             bool withMarkers) {
    ServerCard *card = startZone->card(startPos);
    if (!card)
        return;

    auto stage = zone("stage");
    auto markerBearer = stage->card(targetPos);
    if (!markerBearer)
        return;

    if (startZone->name() == "stage" || startZone->name() == "climax" || startZone->name() == "hand") {
        // revert effects of cont abilities
        playContAbilities(card, true/*revert*/);
    }

    auto markers = std::move(card->markers());
    auto cardPtr = startZone->takeCard(startPos);
    if (!cardPtr)
        return;

    cardPtr->reset();

    if (withMarkers) {
        transferMarkers(markers, startPos, targetPos, markerBearer, faceOrientation);
    }
    card = markerBearer->addMarker(std::move(cardPtr));

    EventMoveCard event;
    event.set_start_zone(startZone->name());
    event.set_start_pos(startPos);
    event.set_target_zone("marker");
    event.set_target_pos(targetPos);
    if (startZone->name() == "stage" && markers.size()) {
        if (!withMarkers) {
            auto movedMarkers = moveMarkersToWr(markers);
            *event.mutable_markers() = { movedMarkers.begin(), movedMarkers.end() };
        }
    }

    if (faceOrientation == asn::FaceOrientation::FaceUp) {
        event.set_code(card->code());
        event.set_card_id(card->id());
    }

    sendToBoth(event);

    mGame->resolveAllContAbilities();
}

void ServerPlayer::transferMarkers(std::vector<std::unique_ptr<ServerCard>> &markers, int startPos, int targetPos,
                                   ServerCard *markerBearer, asn::FaceOrientation faceOrientation) {
    if (markers.empty())
        return;

    for (size_t i = markers.size() - 1; i >= 0; --i) {
        auto marker = markerBearer->addMarker(std::move(markers[i]));

        EventMoveCard event;
        event.set_start_zone("marker");
        event.set_start_pos(startPos);
        event.set_target_zone("marker");
        event.set_target_pos(targetPos);
        event.set_marker_pos(i);
        if (faceOrientation == asn::FaceOrientation::FaceUp) {
            event.set_code(marker->code());
            event.set_card_id(marker->id());
        }
        sendToBoth(event);
    }

    markers.clear();
}

ServerCard* ServerPlayer::removeMarker(ServerCard *markerBearer, int markerPos,
                                       const asn::Place &place, int targetPos) {
    auto markerPtr = markerBearer->takeMarker(markerPos);
    if (!markerPtr)
        return nullptr;

    std::string targetZoneName = std::string(asnZoneToString(place.zone));
    auto pzone = zone(targetZoneName);

    if (place.zone == asn::Zone::Stage) {
        moveCardToStage(std::move(markerPtr), "marker", markerBearer->pos(), pzone, targetPos, markerPos);
        return nullptr;
    }

    auto faceOrientation = markerPtr->faceOrientation();
    markerPtr->reset();
    auto card = pzone->addCard(std::move(markerPtr));

    EventMoveCard eventPublic;
    eventPublic.set_start_zone("marker");
    eventPublic.set_start_pos(markerBearer->pos());
    eventPublic.set_target_zone(targetZoneName);
    eventPublic.set_target_pos(-1);

    if (!(faceOrientation == asn::FaceOrientation::FaceDown && pzone->type() == ZoneType::HiddenZone)) {
        eventPublic.set_card_id(card->id());
        eventPublic.set_code(card->code());
    }

    EventMoveCard eventPrivate(eventPublic);
    if (faceOrientation == asn::FaceOrientation::FaceDown && pzone->type() == ZoneType::PrivateZone) {
        eventPrivate.set_card_id(card->id());
        eventPrivate.set_code(card->code());
    }

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    mGame->resolveAllContAbilities();

    return card;
}

Resumable ServerPlayer::processClockPhaseResult(CommandClockPhase cmd) {
    if (cmd.count()) {
        moveCard("hand", cmd.card_pos(), "clock");
        if (zone("clock")->count() >= 7)
            co_await levelUp();
        co_await drawCards(2);
    }

    co_await mGame->checkTiming();

    mGame->setPhase(asn::Phase::MainPhase);
    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::MainPhase);
    co_await mGame->checkTiming();

    sendToBoth(EventMainPhase());
    sendPlayableCards();

    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::descriptor()->name());
    addExpectedCommand(CommandPlayAct::descriptor()->name());
    addExpectedCommand(CommandClimaxPhase::descriptor()->name());
    addExpectedCommand(CommandSwitchStagePositions::descriptor()->name());
}

Resumable ServerPlayer::playCard(const CommandPlayCard cmd) {
    auto *hand = zone("hand");
    auto cardPtr = hand->card(cmd.hand_pos());
    if (!cardPtr)
        co_return;

    if (cardPtr->type() == CardType::Char)
        co_await playCharacter(cmd);
    else if (cardPtr->type() == CardType::Climax)
        co_await playClimax(cmd.hand_pos());
    else if (cardPtr->type() == CardType::Event)
        co_await playEvent(cmd.hand_pos());

    sendPlayableCards();
}

Resumable ServerPlayer::playCounter(const CommandPlayCounter cmd) {
    auto hand = zone("hand");
    auto card = hand->card(cmd.hand_pos());
    if (!card)
        co_return;

    if (card->type() == CardType::Event) {
        co_await playEvent(cmd.hand_pos());
        co_await mGame->continueFromDamageStep();
        co_return;
    }

    if (mCannotPlayBackups)
        co_return;

    clearExpectedComands();

    triggerBackupAbility(card);
    co_await mGame->checkTiming();

    co_await mGame->continueFromDamageStep();
}

Resumable ServerPlayer::playCharacter(const CommandPlayCard cmd) {
    if (mGame->phase() != asn::Phase::MainPhase)
        co_return;

    ServerCardZone *hand = zone("hand");
    ServerCardZone *stage = zone("stage");
    if (cmd.stage_pos() >= stage->count())
        co_return;

    auto cardPtr = hand->card(cmd.hand_pos());
    if (!cardPtr)
        co_return;

    if (!canPlay(cardPtr))
        co_return;

    // assuming cont abilities that take effect while in hand don't affect other cards
    cardPtr->reset();
    checkZoneChangeTrigger(cardPtr, "hand", "stage");

    auto currentStageCard = stage->card(cmd.stage_pos());
    if (currentStageCard) {
        checkZoneChangeTrigger(currentStageCard, "stage", "wr");
        // revert effects of cont abilities
        playContAbilities(currentStageCard, true);
        currentStageCard->reset();
    }

    auto card = hand->takeCard(cmd.hand_pos());

    auto oldStageCard = stage->putOnStage(std::move(card), cmd.stage_pos());
    auto cardInPlay = stage->card(cmd.stage_pos());
    cardInPlay->setFirstTurn(true);

    EventPlayCard event;
    event.set_card_id(cardInPlay->id());
    event.set_hand_pos(cmd.hand_pos());
    event.set_stage_pos(cmd.stage_pos());
    event.set_code(cardInPlay->code());

    sendToBoth(event);

    if (oldStageCard)
        zone("wr")->addCard(std::move(oldStageCard));

    auto stock = zone("stock");
    for (int i = 0; i < cardInPlay->cost(); ++i)
        moveCard("stock", stock->count() - 1, "wr");
    mGame->resolveAllContAbilities();
    checkOnPlayTrigger(cardInPlay);
    co_await mGame->checkTiming();
}

Resumable ServerPlayer::playClimax(int handIndex) {
    if (mGame->phase() != asn::Phase::ClimaxPhase)
        co_return;

    auto hand = zone("hand");
    auto climaxZone = zone("climax");

    auto card = hand->takeCard(handIndex);
    if (!card)
        co_return;
    auto cardPtr = climaxZone->addCard(std::move(card));

    EventPlayCard event;
    event.set_hand_pos(static_cast<google::protobuf::uint32>(handIndex));
    event.set_code(cardPtr->code());
    sendToBoth(event);

    mGame->resolveAllContAbilities();

    checkOnPlayTrigger(cardPtr);
    checkZoneChangeTrigger(cardPtr, "hand", "climax");
    co_await mGame->checkTiming();

    co_await startAttackPhase();
}

Resumable ServerPlayer::playEvent(int handIndex) {
    if (mCannotPlayEvents)
        co_return;

    auto hand = zone("hand");
    auto resZone = zone("res");
    auto stock = zone("stock");

    auto card = hand->card(handIndex);
    if (!canPlay(card))
        co_return;

    moveCard("hand", handIndex, "res");

    if (card->cost() > stock->count())
        mForcedCostReduction = card->cost() - stock->count();
    mGame->triggerManager()->payingCostEvent(card);
    co_await limitedCheckTiming();

    int cost = card->cost() - costReduction();
    resetCostReduction();
    for (int i = 0; i < cost; ++i)
        moveCard("stock", stock->count() - 1, "wr");

    co_await playEventEffects(card);

    if (card->zone()->name() == "res")
        moveCard("res", card->pos(), "wr");

    checkOnPlayTrigger(card);

    co_await mGame->checkTiming();
}

Resumable ServerPlayer::switchPositions(const CommandSwitchStagePositions cmd) {
    ServerCardZone *stage = zone("stage");
    if (cmd.stage_pos_from() >= stage->count()
        || cmd.stage_pos_to() >= stage->count())
        co_return;

    switchPositions(cmd.stage_pos_from(), cmd.stage_pos_to());

    co_await mGame->checkTiming();
}

void ServerPlayer::switchPositions(int from, int to) {
    ServerCardZone *stage = zone("stage");
    auto card1 = stage->card(from);
    auto card2 = stage->card(to);

    if ((card1 && card1->cannotMove()) ||
        (card2 && card2->cannotMove()))
        return;

    std::tuple<int, int, int> oldAttrs1;
    std::tuple<int, int, int> oldAttrs2;
    if (card1) {
        oldAttrs1 = card1->attributes();
        card1->buffManager()->removePositionalContBuffs();
        mGame->removePositionalContBuffsBySource(card1);
    }
    if (card2) {
        oldAttrs2 = card2->attributes();
        card2->buffManager()->removePositionalContBuffs();
        mGame->removePositionalContBuffsBySource(card2);
    }

    stage->switchPositions(from, to);
    EventSwitchStagePositions event;
    event.set_stage_pos_from(from);
    event.set_stage_pos_to(to);
    sendToBoth(event);

    mGame->resolveAllContAbilities();
    if (card1)
        card1->buffManager()->sendChangedAttrs(oldAttrs1);
    if (card2)
        card2->buffManager()->sendChangedAttrs(oldAttrs2);
}

void ServerPlayer::swapCards(ServerCard *card1, ServerCard *card2) {
    int card1Pos = card1->pos();
    int card2Pos = card2->pos();
    auto zone1 = card1->zone();
    auto zone2 = card2->zone();
    auto cardHolder1 = zone1->takeCard(card1->pos());
    auto cardHolder2 = zone2->takeCard(card2->pos());
    if (zone1->name() == "stage")
        zone1->putOnStage(std::move(cardHolder2), card1Pos);
    else
        zone1->addCard(std::move(cardHolder2), card1Pos);
    if (zone2->name() == "stage")
        zone2->putOnStage(std::move(cardHolder1), card2Pos);
    else
        zone2->addCard(std::move(cardHolder1), card2Pos);
}

bool ServerPlayer::canPlay(ServerCard *card) {
    if (card->cannotPlay())
        return false;
    if (mGame->phase() == asn::Phase::ClimaxPhase && card->type() != CardType::Climax)
        return false;
    if (card->level() > mLevel)
        return false;
    int stockAvailable = zone("stock")->count() + stockCostSubstitution(card);
    if (static_cast<int>(card->cost()) > stockAvailable)
        return false;
    if (!card->canPlayWithoutColorRequirement() &&
        (card->level() > 0 || card->type() == CardType::Climax)) {
        if (!zone("clock")->hasCardWithColor(card->color())
            && !zone("level")->hasCardWithColor(card->color()))
            return false;
    }
    return true;
}

bool ServerPlayer::canPlayCounter(ServerCard *card) {
    if (!card->isCounter())
        return false;
    if (card->level() > mLevel)
        return false;
    if (card->type() == CardType::Char) {
        // TODO: move the check from client to the server side
    } else if (card->type() == CardType::Event) {
        if (mCannotPlayEvents)
            return false;
        int stockAvailable = zone("stock")->count() + stockCostSubstitution(card);
        if (card->cost() > stockAvailable)
            return false;
        return true;
    }

    return false;
}

Resumable ServerPlayer::climaxPhase() {
    mGame->setPhase(asn::Phase::ClimaxPhase);
    EventPhaseEvent event;
    event.set_phase(static_cast<int>(asn::Phase::ClimaxPhase));
    sendToBoth(event);

    checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::ClimaxPhase);
    co_await mGame->checkTiming();

    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::descriptor()->name());
    addExpectedCommand(CommandAttackPhase::descriptor()->name());
    sendToBoth(EventClimaxPhase());
}

bool ServerPlayer::canAttack() {
    if (mGame->firstTurn() && mAttacksThisTurn)
        return false;
    auto stage = zone("stage");
    for (int i = 0; i < 3; ++i) {
        if (stage->card(i) && stage->card(i)->state() == asn::State::Standing) {
            return true;
        }
    }
    return false;
}

Resumable ServerPlayer::startAttackPhase() {
    clearExpectedComands();
    co_await startOfAttackPhase();
    if (canAttack())
        attackDeclarationStep();
    else
        co_await mGame->encoreStep();
}

Resumable ServerPlayer::startOfAttackPhase() {
    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::AttackPhase);
    co_await mGame->checkTiming();
}

void ServerPlayer::attackDeclarationStep() {
    mGame->setPhase(asn::Phase::AttackDeclarationStep);
    addExpectedCommand(CommandDeclareAttack::descriptor()->name());
    addExpectedCommand(CommandEncoreStep::descriptor()->name());
    sendToBoth(EventAttackDeclarationStep());
}

bool isCenterStagePosition(int pos) { return pos >= 3 ? false : true; }
Resumable ServerPlayer::declareAttack(const CommandDeclareAttack cmd) {
    auto stage = zone("stage");
    if (!isCenterStagePosition(cmd.stage_pos()))
        co_return;
    auto attCard = stage->card(cmd.stage_pos());
    if (!attCard || attCard->state() != asn::State::Standing)
        co_return;

    AttackType type = cmd.attack_type();
    auto battleOpp = oppositeCard(attCard);
    if (!battleOpp)
        type = AttackType::DirectAttack;

    if (type == AttackType::FrontAttack && attCard->cannotFrontAttack())
        co_return;
    if (type == AttackType::SideAttack && attCard->cannotSideAttack())
        co_return;

    clearExpectedComands();

    attCard->setState(asn::State::Rested);
    setAttackingCard(attCard);
    mAttacksThisTurn++;

    setAttackType(type);
    EventDeclareAttack event;
    event.set_stage_pos(cmd.stage_pos());
    event.set_attack_type(type);
    sendToBoth(event);

    if (type == AttackType::DirectAttack) {
        attCard->buffManager()->addAttributeBuff(asn::AttributeType::Soul, 1);
    } else if (type == AttackType::SideAttack) {
        if (!attCard->sideAttackWithoutPenalty() && battleOpp->level() > 0)
            attCard->buffManager()->addAttributeBuff(asn::AttributeType::Soul, -battleOpp->level());
    }

    if (type == AttackType::FrontAttack) {
        attCard->setInBattle(true);
        battleOpp->setInBattle(true);

        // for 'in battles involving this card'
        mGame->resolveAllContAbilities();
    }

    checkOnAttack(attCard);
    if (battleOpp)
        battleOpp->player()->checkOnBeingAttacked(battleOpp,
                                                  protoAttackTypeToAttackType(type));
    co_await mGame->checkTiming();

    co_await triggerStep(cmd.stage_pos());
}

Resumable ServerPlayer::triggerStep(int pos) {
    sendPhaseEvent(asn::Phase::TriggerStep);

    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::TriggerStep);
    co_await mGame->checkTiming();

    co_await performTriggerStep(pos);
    if (mAttackingCard->triggerCheckTwice()) {
        co_await performTriggerStep(pos);
        mAttackingCard->setTriggerCheckTwice(false);
    }

    co_await mGame->checkTiming();

    if (attackType() == FrontAttack)
        co_await mGame->opponentOfPlayer(mId)->counterStep();
    else
        co_await mGame->continueFromDamageStep();
}

Resumable ServerPlayer::performTriggerStep(int pos) {
    co_await moveTopDeck("res");
    auto card = zone("res")->card(0);
    // end of game
    if (!card)
        co_await std::suspend_always();
    checkOnTriggerReveal(card);
    for (auto trigger: card->triggers()) {
        if (trigger == TriggerIcon::Soul && attackingCard())
            attackingCard()->buffManager()->addAttributeBuff(asn::AttributeType::Soul, 1);
        else if (trigger != TriggerIcon::Soul)
            co_await resolveTrigger(card, trigger);
    }
    moveCard("res", 0, "stock");
}

Resumable ServerPlayer::counterStep() {
    mGame->setPhase(asn::Phase::CounterStep);

    addExpectedCommand(CommandTakeDamage::descriptor()->name());
    addExpectedCommand(CommandPlayCounter::descriptor()->name());
    sendToBoth(EventCounterStep());

    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::CounterStep);
    co_await mGame->checkTiming();

    sendPlayableCards();
}

Resumable ServerPlayer::damageStep() {
    mGame->setPhase(asn::Phase::DamageStep);
    clearExpectedComands();

    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::DamageStep);
    co_await mGame->checkTiming();

    auto attCard = getOpponent()->attackingCard();
    if (!attCard)
        co_return;

    if (attCard->zone()->name() != "stage")
        co_return;

    co_await takeDamage(attCard->soul(), attCard);

    co_await mGame->checkTiming();
}

Resumable ServerPlayer::endOfAttack(bool forced) {
    auto attCard = attackingCard();
    if (!forced) {
        if (attCard)
            triggerOnEndOfCardsAttack(attCard);

        co_await mGame->checkTiming();
    }

    if (attackType() == AttackType::FrontAttack) {
        if (attCard)
            attCard->setInBattle(false);

        auto oppStage = getOpponent()->zone("stage");
        for (int i = 0; i < oppStage->count(); ++i) {
            auto card = oppStage->card(i);
            if (card)
                card->setInBattle(false);
        }
    }

    // for 'in battles involving this card'
    mGame->resolveAllContAbilities();

    setAttackingCard(nullptr);
    sendToBoth(EventEndOfAttack());
}

Resumable ServerPlayer::levelUp() {
    auto clock = zone("clock");
    if (clock->count() < 7)
        co_return;

    clearExpectedComands();

    if (zone("level")->count() == 3) {
        sendEndGame(false);
        co_await std::suspend_always();
    }
    addExpectedCommand(CommandLevelUp::descriptor()->name());
    sendToBoth(EventLevelUp());

    auto cmd = co_await waitForCommand();
    if (!cmd.command().Is<CommandLevelUp>())
        co_return;

    CommandLevelUp lvlCmd;
    cmd.command().UnpackTo(&lvlCmd);
    if (lvlCmd.clock_pos() > 6)
        co_return;

    if (!moveCard("clock", lvlCmd.clock_pos(), "level"))
        co_return;

    clearExpectedComands();

    mLevel++;
    auto wr = zone("wr");
    for (int i = 0; i < 6; ++i) {
        auto card = clock->takeCard(0);
        wr->addCard(std::move(card));
    }

    sendToBoth(EventClockToWr());

    if (clock->count() > 6)
        co_await levelUp();
}

Resumable ServerPlayer::encoreStep() {
    clearExpectedComands();
    // send for synchronization
    EventPhaseEvent event;
    event.set_phase(static_cast<int>(asn::Phase::EncoreStep));
    sendToBoth(event);

    auto stage = zone("stage");
    auto needEncore = [stage]() {
        for (int i = 0; i < 5; ++i) {
            auto card = stage->card(i);
            if (card && card->state() == asn::State::Reversed)
                return true;
        }
        return false;
    };

    if (needEncore()) {
        addExpectedCommand(CommandEncoreCharacter::descriptor()->name());
        addExpectedCommand(CommandEndTurn::descriptor()->name());

        while (true) {
            sendToBoth(EventEncoreStep());
            auto cmd = co_await waitForCommand();

            if (cmd.command().Is<CommandEncoreCharacter>()) {
                CommandEncoreCharacter encoreCmd;
                cmd.command().UnpackTo(&encoreCmd);
                co_await encoreCharacter(encoreCmd);
                if (!needEncore())
                    break;
            } else if (cmd.command().Is<CommandEndTurn>()) {
                for (int i = 0; i < 5; ++i) {
                    auto card = stage->card(i);
                    if (card && card->state() == asn::State::Reversed) {
                        moveCard("stage", i, "wr", -1, false, false);
                        co_await mGame->checkTiming();
                    }
                }
                break;
            }
        }

        clearExpectedComands();
    }
}

Resumable ServerPlayer::encoreCharacter(const CommandEncoreCharacter cmd) {
    if (cmd.stage_pos() < 0 || cmd.stage_pos() >= 5)
        co_return;

    auto stage = zone("stage");
    if (!stage->card(cmd.stage_pos())
        || stage->card(cmd.stage_pos())->state() != asn::State::Reversed)
        co_return;

    moveCard("stage", cmd.stage_pos(), "wr");

    co_await mGame->checkTiming();
}

Resumable ServerPlayer::discardDownTo7() {
    clearExpectedComands();
    if (mActive) {
        auto hand = zone("hand");
        while (hand->count() > 7) {
            sendToBoth(EventDiscardDownTo7());
            addExpectedCommand(CommandMoveCard::descriptor()->name());
            auto cmd = co_await waitForCommand();

            if (cmd.command().Is<CommandMoveCard>()) {
                CommandMoveCard moveCmd;
                cmd.command().UnpackTo(&moveCmd);
                if (moveCmd.start_zone() != "hand" || moveCmd.target_zone() != "wr")
                    continue;
                moveCard("hand", moveCmd.start_pos(), "wr");
            }
        }
        clearExpectedComands();
    }
}

void ServerPlayer::clearClimaxZone() {
    auto climax = zone("climax");
    if (climax->count() > 0)
        moveCard("climax", 0, "wr");
}

Resumable ServerPlayer::refresh() {
    int count = zone("wr")->count();
    if (!count) {
        sendEndGame(false);
        co_return;
    }
    moveWrToDeck();
    sendToBoth(EventRefresh());

    co_await moveTopDeck("clock");
}

void ServerPlayer::moveWrToDeck() {
    auto deck = zone("deck");
    auto wr = zone("wr");
    int count = wr->count();
    for (int i = 0; i < count; ++i) {
        auto card = wr->takeTopCard();
        deck->addCard(std::move(card));
    }
    deck->shuffle();
}

void ServerPlayer::sendPhaseEvent(asn::Phase phase) {
    EventPhaseEvent event;
    event.set_phase(static_cast<int>(phase));
    sendToBoth(event);
}

void ServerPlayer::sendEndGame(bool victory) {
    mGame->setEnded();

    EventGameEnded event;
    event.set_victory(victory);
    sendGameEvent(event);

    EventGameEnded eventOpp;
    eventOpp.set_victory(!victory);
    mGame->opponentOfPlayer(mId)->sendGameEvent(eventOpp);
}

Resumable ServerPlayer::processPlayActCmd(const CommandPlayAct cmd) {
    auto stage = zone("stage");
    if (cmd.card_pos() >= stage->count())
        co_return;

    auto card = stage->card(cmd.card_pos());
    if (!card)
        co_return;

    if (static_cast<size_t>(cmd.ability_id()) >= card->abilities().size())
        co_return;

    TriggeredAbility ta;
    ta.card = CardImprint(card->zone()->name(), card);
    ta.type = ProtoCard;
    ta.abilityId = cmd.ability_id();
    mQueue.push_back(ta);

    co_await mGame->checkTiming();

    sendPlayableCards();
}

void ServerPlayer::reorderTopCards(const CommandMoveInOrder &cmd, asn::Zone destZone){
    auto pzone = zone(destZone);
    if (pzone->count() < cmd.codes_size() || !cmd.codes_size())
        return;
    for (int i = 0; i < cmd.codes_size() - 1; ++i) {
        for (int j = i; j < pzone->count(); ++j) {
            if (pzone->card(pzone->count() - 1 - j)->code() == cmd.codes(i)) {
                if (i != j)
                    pzone->switchPositions(pzone->count() - 1 - i, pzone->count() - 1 - j);
                break;
            }
        }
    }
}

void ServerPlayer::setCardState(ServerCard *card, asn::State state) {
    if (card->state() == state)
        return;
    card->setState(state);

    EventSetCardState event;
    event.set_stage_pos(card->pos());
    event.set_state(stateToProtoState(state));
    sendToBoth(event);
}

ServerCard *ServerPlayer::oppositeCard(const ServerCard *card) const {
    auto opponent = mGame->opponentOfPlayer(mId);
    if (!opponent)
        return nullptr;

    if (card->pos() > 2)
        return nullptr;

    return opponent->zone("stage")->card(card->pos());
}

ServerPlayer* ServerPlayer::getOpponent() {
    return mGame->opponentOfPlayer(mId);
}

ServerCard *ServerPlayer::cardInBattle() {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;
        if (card->inBattle())
            return card;
    }
    return nullptr;
}

void ServerPlayer::changeAttribute(PlayerAttrType type, bool value) {
    bool oldValue = attribute(type);
    switch (type) {
    case PlayerAttrType::CharAutoCannotDealDamage:
        mCharAutoCannotDealDamage = value;
        break;
    case PlayerAttrType::CannotPlayBackups:
        mCannotPlayBackups = value;
        break;
    case PlayerAttrType::CannotPlayEvents:
        mCannotPlayEvents = value;
        break;
    default:
        assert(false);
    }
    if (oldValue != value)
        sendPlayerAttrChange(type, value);
}

bool ServerPlayer::attribute(PlayerAttrType type) const {
    switch (type) {
    case PlayerAttrType::CharAutoCannotDealDamage:
        return mCharAutoCannotDealDamage;
    case PlayerAttrType::CannotPlayBackups:
        return mCannotPlayBackups;
    case PlayerAttrType::CannotPlayEvents:
        return mCannotPlayEvents;
    }
    assert(false);
    return false;
}

Resumable ServerPlayer::takeDamage(int damage, ServerCard *attacker) {
    auto resZone = zone("res");
    bool cancelled = false;
    for (int i = 0; i < damage; ++i) {
        co_await moveTopDeck("res");
        auto card = resZone->topCard();
        // it should be the end of the game since deck AND wr is empty
        // or some unrecoverable error, so halt execution
        if (!card)
            co_await std::suspend_always();
        if (card->type() == CardType::Climax) {
            cancelled = true;
            break;
        }
    }

    while (resZone->card(0))
        moveCard("res", 0, cancelled ? "wr" : "clock");

    if (zone("clock")->count() >= 7)
        co_await levelUp();

    attacker->setPreviousDamage(damage);
    getOpponent()->checkOnDamageCancel(attacker, cancelled);
    checkOnDamageTakenCancel(cancelled);
    mGame->triggerManager()->damageCancelEvent(attacker, cancelled);
    mGame->clearShotTrigger();
}

Resumable ServerPlayer::checkRefreshAndLevelUp() {
    bool needRefresh = zone("deck")->count() == 0;
    bool needLevelUp = zone("clock")->count() >= 7;
    if (needRefresh && needLevelUp) {
        sendToBoth(EventRuleActionChoice());

        clearExpectedComands();
        addExpectedCommand(CommandChoice::descriptor()->name());

        int choice;
        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                choice = choiceCmd.choice();
                break;
            }
        }
        clearExpectedComands();

        if (choice == 0) {
            co_await refresh();
            co_await levelUp();
        } else {
            co_await levelUp();
            co_await refresh();
        }
        co_return;
    }

    if (needRefresh)
        co_await refresh();
    if (needLevelUp)
        co_await levelUp();
}

void ServerPlayer::sendPlayerAttrChange(PlayerAttrType type, bool value) {
    EventSetPlayerAttr event;
    event.set_attr(getProtoPlayerAttrType(type));
    event.set_value(value);
    sendToBoth(event);
}

int ServerPlayer::stockCostSubstitution(ServerCard *card) {
    int result = 0;
    auto &abilities = mGame->delayedAbilities();
    for (const auto &delayedAbility: abilities) {
        if (delayedAbility.thisCard.card->player()->id() != mId)
            continue;
        const auto &ability = delayedAbility.ability;
        for (const auto &trigger: ability.triggers) {
            if (trigger.type != asn::TriggerType::OnPayingCost)
                continue;
            const auto &t = std::get<asn::OnPayingCostTrigger>(trigger.trigger);
            if (!checkCardMatches(card, t.target, delayedAbility.thisCard.card))
                continue;
            if (ability.effects.empty())
                continue;
            const auto &effect = ability.effects.front();
            if (effect.type != asn::EffectType::CostSubstitution)
                continue;
            const auto &costEffect = std::get<asn::CostSubstitution>(effect.effect);
            if (!costEffect.effect)
                continue;
            AbilityPlayer abilityPlayer(this);
            abilityPlayer.setThisCard(delayedAbility.thisCard);
            result += abilityPlayer.timesCanBePerformed(*costEffect.effect);
            break;
        }
    }
    return result;
}

void ServerPlayer::sendPlayableCards() {
    if (mGame->phase() != asn::Phase::MainPhase &&
        mGame->phase() != asn::Phase::CounterStep)
        return;
    EventPlayableCards event;
    for (int i = 0; i < zone(asn::Zone::Hand)->count(); ++i) {
        auto card = zone(asn::Zone::Hand)->card(i);
        if (mGame->phase() == asn::Phase::CounterStep) {
            if (canPlayCounter(card))
                event.add_hand_pos(i);
        } else if (canPlay(card)) {
            event.add_hand_pos(i);
        }
    }
    sendGameEvent(event);
}

asn::Ability TriggeredAbility::getAbility() const {
    if (ability)
        return *ability;
    if (type == ProtoCard && card.card)
        return card.card->abilities()[abilityId].ability;
    else if (type == ProtoGlobal)
        return globalAbility(static_cast<GlobalAbility>(abilityId));
    return {};
}

Resumable ServerPlayer::testAction() {
    moveCard("deck", zone("deck")->count() - 1, "clock");
    co_await checkRefreshAndLevelUp();
}
