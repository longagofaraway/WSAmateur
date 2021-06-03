#include "serverPlayer.h"

#include <algorithm>

#include "abilities.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include "abilityUtils.h"
#include "cardDatabase.h"
#include "globalAbilities/globalAbilities.h"
#include "serverCardZone.h"
#include "serverGame.h"
#include "serverProtocolHandler.h"
#include "serverStage.h"

#include <QDebug>

ServerPlayer::ServerPlayer(ServerGame *game, ServerProtocolHandler *client, int id)
    : mGame(game), mClient(client), mId(id) { }

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
        switchPositions(switchCmd);
    } else if (cmd.command().Is<CommandClimaxPhase>()) {
        climaxPhase();
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
    mClient->sendGameEvent(event, playerId);
}

void ServerPlayer::sendToBoth(const google::protobuf::Message &event) {
    sendGameEvent(event);
    mGame->sendPublicEvent(event, mId);
}

void ServerPlayer::addDeck(const std::string &deck) {
    mDeck = std::make_unique<DeckList>(deck);
    mExpectedCommands.push_back(CommandReadyToStart::descriptor()->name());
}

ServerCardZone* ServerPlayer::addZone(std::string_view name, ZoneType type) {
    return mZones.emplace(name, std::make_unique<ServerCardZone>(this, name, type)).first->second.get();
}

ServerCardZone* ServerPlayer::zone(std::string_view name) {
    if (!mZones.count(name))
        return nullptr;

    return mZones.at(name).get();
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

    for (auto &card: mDeck->cards()) {
        auto cardInfo = CardDatabase::get().getCard(card.code);
        for (int i = 0; i < card.count; ++i)
            deck->addCard(cardInfo);
    }

    deck->shuffle();
}

void ServerPlayer::createStage() {
    mZones.emplace("stage", std::make_unique<ServerStage>(this));
}

void ServerPlayer::startGame() {
    mExpectedCommands.clear();
    mExpectedCommands.emplace_back(CommandMulligan::descriptor()->name(), 1);
}

Resumable ServerPlayer::startTurn() {
    sendToBoth(EventStartTurn());

    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (card && card->state() != StateStanding)
            setCardState(card, StateStanding);
    }

    mGame->checkPhaseTrigger(asn::PhaseState::Start, asn::Phase::DrawPhase);
    co_await mGame->checkTiming();

    drawCards(1);
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
    eventPrivate.set_firstturn(mActive);
    EventInitialHand eventPublic(eventPrivate);
    for (int i = 0; i < 5; ++i) {
        auto card = deck->takeTopCard();
        if (!card)
            continue;
        auto code = eventPrivate.add_codes();
        *code = card->code();
        hand->addCard(std::move(card));
    }

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);
}

Resumable ServerPlayer::mulligan(const CommandMulligan &cmd) {
    if (cmd.ids_size()) {
        std::vector<int> ids;
        for (int i = 0; i < cmd.ids_size(); ++i)
            ids.push_back(cmd.ids(i));

        moveCards("hand", ids, "wr");
        drawCards(static_cast<int>(ids.size()));
    }
    mMulliganFinished = true;
    co_await mGame->endMulligan();
}

void ServerPlayer::drawCards(int number) {
    for (int i = 0; i < number; ++i)
        moveTopDeck("hand");
}

void ServerPlayer::moveCards(std::string_view startZoneName,  const std::vector<int> &cardIds, std::string_view targetZoneName) {
    auto sortedIds = cardIds;
    std::sort(sortedIds.begin(), sortedIds.end());
    for (int i = static_cast<int>(sortedIds.size()); i-- > 0;)
        moveCard(startZoneName, sortedIds[i], targetZoneName);
}

bool ServerPlayer::moveCard(std::string_view startZoneName, int startId, std::string_view targetZoneName,
                            int targetId, bool reveal, bool enableGlobEncore) {
    ServerCardZone *startZone = zone(startZoneName);
    if (!startZone)
        return false;

    ServerCardZone *targetZone = zone(targetZoneName);
    if (!targetZone)
        return false;

    if (startZoneName != "stage" && targetZoneName == "stage")
        return moveCardToStage(startZone, startId, targetZone, targetId);
    if (startZoneName == "stage" && targetZoneName == "stage") {
        switchPositions(startId, targetId);
        return true;
    }

    ServerCard *card = startZone->card(startId);
    if (!card)
        return false;

    if (startZoneName == "stage" || startZoneName == "climax" || startZoneName == "hand") {
        // revert effects of cont abilities
        playContAbilities(card, true);
        card->reset();
    }

    auto cardPtr = startZone->takeCard(startId);

    targetZone->addCard(std::move(cardPtr));
    int newCardId = targetZone->count() - 1;

    EventMoveCard eventPublic;
    eventPublic.set_startid(static_cast<google::protobuf::uint32>(startId));
    eventPublic.set_startzone(startZone->name());
    eventPublic.set_targetzone(targetZone->name());

    if (startZone->type() == ZoneType::HiddenZone && targetZone->type() == ZoneType::PublicZone)
        eventPublic.set_code(card->code());

    EventMoveCard eventPrivate(eventPublic);

    if (startZone->type() == ZoneType::HiddenZone && targetZone->type() == ZoneType::PrivateZone) {
        eventPrivate.set_code(card->code());
        if (reveal)
            eventPublic.set_code(card->code());
    }

    // revealing card from hand, opponent didn't see this card yet
    if (startZone->type() == ZoneType::PrivateZone && targetZone->type() == ZoneType::PublicZone)
        eventPublic.set_code(card->code());

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    mGame->resolveAllContAbilities();

    checkZoneChangeTrigger(card, newCardId, startZoneName, targetZoneName);
    if (enableGlobEncore)
        checkGlobalEncore(card, newCardId, startZoneName, targetZoneName);

    return true;
}

bool ServerPlayer::moveCardToStage(ServerCardZone *startZone, int startId, ServerCardZone *targetZone, int targetId) {
    if (targetId >= 5)
        return false;

    auto card = startZone->takeCard(startId);
    if (!card)
        return false;

    card->reset();

    auto oldStageCard = targetZone->putOnStage(std::move(card), targetId);
    auto cardOnStage = targetZone->card(targetId);

    EventMoveCard event;
    event.set_code(cardOnStage->code());
    event.set_startzone(startZone->name());
    event.set_startid(startId);
    event.set_targetzone(targetZone->name());
    event.set_targetid(targetId);
    sendToBoth(event);

    ServerCard *pOldStageCard = nullptr;
    int oldStageCardId = 0;
    if (oldStageCard) {
        pOldStageCard = zone("wr")->addCard(std::move(oldStageCard));
        oldStageCardId = zone("wr")->count() - 1;
    }

    mGame->resolveAllContAbilities();
    checkZoneChangeTrigger(cardOnStage, targetId, startZone->name(), "stage");
    if (pOldStageCard)
        checkZoneChangeTrigger(pOldStageCard, oldStageCardId, "stage", "wr");

    return true;
}

void ServerPlayer::moveTopDeck(std::string_view targetZoneName) {
    auto deck = zone("deck");
    moveCard("deck", deck->count() - 1, targetZoneName);

    if (deck->count() == 0)
        refresh();
}

Resumable ServerPlayer::processClockPhaseResult(CommandClockPhase cmd) {
    if (cmd.count()) {
        moveCard("hand", cmd.cardid(), "clock");
        if (zone("clock")->count() >= 7)
            co_await levelUp();
        drawCards(2);
    }

    co_await mGame->checkTiming();

    sendToBoth(EventMainPhase());

    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::descriptor()->name());
    addExpectedCommand(CommandPlayAct::descriptor()->name());
    addExpectedCommand(CommandClimaxPhase::descriptor()->name());
    addExpectedCommand(CommandAttackPhase::descriptor()->name());
    addExpectedCommand(CommandSwitchStagePositions::descriptor()->name());
}

Resumable ServerPlayer::playCard(const CommandPlayCard &cmd) {
    auto *hand = zone("hand");
    auto cardPtr = hand->card(cmd.handid());
    if (!cardPtr)
        co_return;

    if (cardPtr->type() == CardType::Char)
        co_await playCharacter(cmd);
    else if (cardPtr->type() == CardType::Climax)
        co_await playClimax(cmd.handid());
    else if (cardPtr->type() == CardType::Event)
        co_await playEvent(cmd.handid());
}

Resumable ServerPlayer::playCounter(const CommandPlayCounter &cmd) {
    if (!mCanPlayBackups)
        co_return;

    auto hand = zone("hand");
    auto card = hand->card(cmd.handid());
    if (!card)
        co_return;

    if (card->type() != CardType::Char)
        co_return;

    clearExpectedComands();

    triggerBackupAbility(card);
    co_await mGame->checkTiming();

    co_await mGame->continueFromDamageStep();
}

Resumable ServerPlayer::playCharacter(const CommandPlayCard &cmd) {
    ServerCardZone *hand = zone("hand");
    ServerCardZone *stage = zone("stage");
    if (cmd.stageid() >= stage->count())
        co_return;

    auto cardPtr = hand->card(cmd.handid());
    if (!cardPtr)
        co_return;

    if (!canPlay(cardPtr))
        co_return;

    // assuming cont abilities that take effect while in hand don't affect other cards
    cardPtr->reset();

    auto card = hand->takeCard(cmd.handid());

    auto oldStageCard = stage->putOnStage(std::move(card), cmd.stageid());
    auto cardInPlay = stage->card(cmd.stageid());

    EventPlayCard eventPublic;
    eventPublic.set_handid(cmd.handid());
    eventPublic.set_stageid(cmd.stageid());
    EventPlayCard eventPrivate(eventPublic);
    eventPublic.set_code(cardInPlay->code());

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    ServerCard *pOldStageCard = nullptr;
    int oldStageCardId = 0;
    if (oldStageCard) {
        pOldStageCard = zone("wr")->addCard(std::move(oldStageCard));
        oldStageCardId = zone("wr")->count() - 1;
    }

    auto stock = zone("stock");
    for (int i = 0; i < cardInPlay->cost(); ++i)
        moveCard("stock", stock->count() - 1, "wr");
    mGame->resolveAllContAbilities();
    checkZoneChangeTrigger(cardInPlay, cmd.stageid(), "hand", "stage");
    if (pOldStageCard)
        checkZoneChangeTrigger(pOldStageCard, oldStageCardId, "stage", "wr");
    co_await mGame->checkTiming();
}

Resumable ServerPlayer::playClimax(int handIndex) {
    auto hand = zone("hand");
    auto climaxZone = zone("climax");

    auto card = hand->takeCard(handIndex);
    if (!card)
        co_return;
    auto cardPtr = climaxZone->addCard(std::move(card));

    EventPlayCard eventPublic;
    eventPublic.set_handid(static_cast<google::protobuf::uint32>(handIndex));
    EventPlayCard eventPrivate(eventPublic);
    eventPublic.set_code(cardPtr->code());

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    mGame->resolveAllContAbilities();

    checkZoneChangeTrigger(cardPtr, 0, "hand", "climax");
    co_await mGame->checkTiming();

    co_await startAttackPhase();
}

Resumable ServerPlayer::playEvent(int handIndex) {
    if (!mCanPlayEvents)
        co_return;

    auto hand = zone("hand");
    auto resZone = zone("res");

    auto card = hand->card(handIndex);

    moveCard("hand", handIndex, "res");

    co_await playEventEffects(card);

    moveCard("res", card->pos(), "wr");

    co_await mGame->checkTiming();
}

void ServerPlayer::switchPositions(const CommandSwitchStagePositions &cmd) {
    ServerCardZone *stage = zone("stage");
    if (cmd.stageidfrom() >= stage->count()
        || cmd.stageidto() >= stage->count())
        return;

    switchPositions(cmd.stageidfrom(), cmd.stageidto());
}

void ServerPlayer::switchPositions(int from, int to) {
    ServerCardZone *stage = zone("stage");
    auto card1 = stage->card(from);
    auto card2 = stage->card(to);
    std::tuple<int, int, int> oldAttrs1;
    std::tuple<int, int, int> oldAttrs2;
    if (card1) {
        oldAttrs1 = card1->attributes();
        card1->removePositionalContBuffs();
        mGame->removePositionalContBuffsBySource(card1);
    }
    if (card2) {
        oldAttrs2 = card2->attributes();
        card2->removePositionalContBuffs();
        mGame->removePositionalContBuffsBySource(card2);
    }

    stage->switchPositions(from, to);
    EventSwitchStagePositions event;
    event.set_stageidfrom(from);
    event.set_stageidto(to);
    sendToBoth(event);

    mGame->resolveAllContAbilities();
    if (card1)
        sendChangedAttrs(card1, oldAttrs1);
    if (card2)
        sendChangedAttrs(card2, oldAttrs2);
}

bool ServerPlayer::canPlay(ServerCard *card) {
    if (card->cannotPlay())
        return false;
    if (mGame->phase() == ServerPhase::Climax && card->type() != CardType::Climax)
        return false;
    if (card->level() > mLevel)
        return false;
    if (static_cast<int>(card->cost()) > zone("stock")->count())
        return false;
    if (card->level() > 0 || card->type() == CardType::Climax) {
        if (!zone("clock")->hasCardWithColor(card->color())
            && !zone("level")->hasCardWithColor(card->color()))
            return false;
    }
    return true;
}

void ServerPlayer::climaxPhase() {
    // check timing at start of climax phase
    mGame->setPhase(ServerPhase::Climax);
    EventPhaseEvent event;
    event.set_phase(static_cast<int>(asn::Phase::ClimaxPhase));
    sendToBoth(event);

    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::descriptor()->name());
    sendToBoth(EventClimaxPhase());
}

bool ServerPlayer::canAttack() {
    auto stage = zone("stage");
    for (int i = 0; i < 3; ++i) {
        if (stage->card(i) && stage->card(i)->state() == StateStanding) {
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
    mGame->setPhase(ServerPhase::AttackDeclarationStep);
    addExpectedCommand(CommandDeclareAttack::descriptor()->name());
    addExpectedCommand(CommandEncoreStep::descriptor()->name());
    sendToBoth(EventAttackDeclarationStep());
}

bool isCenterStagePosition(int pos) { return pos >= 3 ? false : true; }
Resumable ServerPlayer::declareAttack(const CommandDeclareAttack &cmd) {
    auto stage = zone("stage");
    if (!isCenterStagePosition(cmd.stageid()))
        co_return;
    auto attCard = stage->card(cmd.stageid());
    if (!attCard || attCard->state() != StateStanding)
        co_return;

    clearExpectedComands();

    attCard->setState(StateRested);
    setAttackingCard(attCard);

    AttackType type = cmd.attacktype();
    auto battleOpp = oppositeCard(attCard);
    if (!battleOpp)
        type = AttackType::DirectAttack;

    setAttackType(type);
    EventDeclareAttack event;
    event.set_stageid(cmd.stageid());
    event.set_attacktype(type);
    sendToBoth(event);

    if (type == AttackType::DirectAttack)
        addAttributeBuff(attCard, asn::AttributeType::Soul, 1);
    else if (type == AttackType::SideAttack)
        addAttributeBuff(attCard, asn::AttributeType::Soul, -battleOpp->level());

    if (type == AttackType::FrontAttack) {
        attCard->setInBattle(true);
        battleOpp->setInBattle(true);

        // for 'in battles involving this card'
        mGame->resolveAllContAbilities();
    }

    checkOnAttack(attCard);
    co_await mGame->checkTiming();

    co_await triggerStep(cmd.stageid());
}

Resumable ServerPlayer::triggerStep(int pos) {
    sendPhaseEvent(asn::Phase::TriggerStep);

    co_await performTriggerStep(pos);
    if (mAttackingCard->triggerCheckTwice()) {
        co_await performTriggerStep(pos);
        mAttackingCard->setTriggerCheckTwice(false);
    }

    co_await mGame->checkTiming();

    if (attackType() == FrontAttack)
        mGame->opponentOfPlayer(mId)->counterStep();
    else
        co_await mGame->continueFromDamageStep();
}

Resumable ServerPlayer::performTriggerStep(int pos) {
    moveTopDeck("res");
    auto card = zone("res")->card(0);
    // end of game
    if (!card)
        co_await std::suspend_always();
    checkOnTriggerReveal(card);
    for (auto trigger: card->triggers()) {
        if (trigger == TriggerIcon::Soul)
            addAttributeBuff(attackingCard(), asn::AttributeType::Soul, 1);
        else
            co_await resolveTrigger(card, trigger);
    }
    moveCard("res", 0, "stock");
}

void ServerPlayer::counterStep() {
    addExpectedCommand(CommandTakeDamage::descriptor()->name());
    addExpectedCommand(CommandPlayCounter::descriptor()->name());
    sendToBoth(EventCounterStep());
}

Resumable ServerPlayer::damageStep() {
    mGame->setPhase(ServerPhase::DamageStep);

    clearExpectedComands();
    auto attCard = getOpponent()->attackingCard();
    if (!attCard)
        co_return;

    if (attCard->zone()->name() != "stage")
        co_return;

    co_await takeDamage(attCard->soul());

    co_await mGame->checkTiming();
}

void ServerPlayer::endOfAttack() {
    if (attackType() == AttackType::FrontAttack) {
        // assuming attacking card cannot be removed from the stage during attack
        auto attCard = attackingCard();
        if (attCard) {
            attCard->setInBattle(false);
            auto battleOpp = oppositeCard(attCard);
            if (battleOpp)
                battleOpp->setInBattle(false);

            // for 'in battles involving this card'
            mGame->resolveAllContAbilities();
        }
    }

    setAttackingCard(nullptr);
    sendToBoth(EventEndOfAttack());
}

Resumable ServerPlayer::levelUp() {
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
    if (lvlCmd.clockid() > 6)
        co_return;

    auto clock = zone("clock");
    if (clock->count() < 7)
        co_return;

    if (!moveCard("clock", lvlCmd.clockid(), "level"))
        co_return;

    clearExpectedComands();

    mLevel++;
    auto wr = zone("wr");
    for (int i = 0; i < 6; ++i) {
        auto card = clock->takeCard(0);
        wr->addCard(std::move(card));
    }

    sendToBoth(EventClockToWr());
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
            if (card && card->state() == StateReversed)
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
                    if (card && card->state() == StateReversed) {
                        moveCard("stage", i, "wr", 0, false, false);
                        co_await mGame->checkTiming();
                    }
                }
                break;
            }
        }

        clearExpectedComands();
    }
}

Resumable ServerPlayer::encoreCharacter(const CommandEncoreCharacter &cmd) {
    if (cmd.stageid() < 0 || cmd.stageid() >= 5)
        co_return;

    auto stage = zone("stage");
    if (!stage->card(cmd.stageid())
        || stage->card(cmd.stageid())->state() != StateReversed)
        co_return;

    moveCard("stage", cmd.stageid(), "wr");

    co_await mGame->checkTiming();
}

Resumable ServerPlayer::endPhase() {
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
                if (moveCmd.startzone() != "hand" || moveCmd.targetzone() != "wr")
                    continue;
                moveCard("hand", moveCmd.startid(), "wr");
            }
        }
        clearExpectedComands();
    }

    auto climax = zone("climax");
    if (climax->count() > 0)
        moveCard("climax", 0, "wr");

    co_await mGame->checkTiming();

    endOfTurnEffectValidation();
}

void ServerPlayer::refresh() {
    int count = zone("wr")->count();
    if (!count) {
        sendEndGame(false);
        return;
    }
    moveWrToDeck();
    sendToBoth(EventRefresh());

    TriggeredAbility a;
    a.type = ProtoRuleAction;
    a.abilityId = static_cast<int>(RuleAction::RefreshPoint);
    mQueue.push_back(a);
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

Resumable ServerPlayer::processPlayActCmd(const CommandPlayAct &cmd) {
    auto stage = zone("stage");
    if (cmd.cardid() >= stage->count())
        co_return;

    auto card = stage->card(cmd.cardid());
    if (!card)
        co_return;

    if (static_cast<size_t>(cmd.abilityid()) >= card->abilities().size())
        co_return;

    TriggeredAbility ta;
    ta.card = CardImprint(card->zone()->name(), cmd.cardid(), card);
    ta.type = ProtoCard;
    ta.abilityId = cmd.abilityid();
    mQueue.push_back(ta);

    co_await mGame->checkTiming();
}

void ServerPlayer::reorderTopCards(const CommandMoveInOrder &cmd, asn::Zone destZone){
    auto pzone = zone(asnZoneToString(destZone));
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

void ServerPlayer::addAbilityToCard(ServerCard *card, const asn::Ability &a, int duration) {
    card->addAbility(a, duration);
    EventAbilityGain ev;
    ev.set_zone(card->zone()->name());
    ev.set_cardid(card->pos());

    std::vector<uint8_t> abilityBuf = encodeAbility(a);
    ev.set_ability(abilityBuf.data(), abilityBuf.size());
    sendToBoth(ev);

    if (a.type == asn::AbilityType::Cont)
        playContAbilities(card);
}

void ServerPlayer::setCardState(ServerCard *card, CardState state) {
    if (card->state() == state)
        return;
    card->setState(state);

    EventSetCardState event;
    event.set_stageid(card->pos());
    event.set_state(state);
    sendToBoth(event);
}

ServerCard *ServerPlayer::oppositeCard(ServerCard *card) const {
    auto opponent = mGame->opponentOfPlayer(mId);
    if (!opponent)
        return nullptr;

    return opponent->zone("stage")->card(card->pos());
}

ServerPlayer* ServerPlayer::getOpponent() {
    return mGame->opponentOfPlayer(mId);
}

Resumable ServerPlayer::takeDamage(int damage) {
    auto resZone = zone("res");
    bool canceled = false;
    for (int i = 0; i < damage; ++i) {
        moveTopDeck("res");
        auto card = resZone->topCard();
        // it should be the end of the game since deck AND wr is empty
        // or some unrecoverable error, so halt execution
        if (!card)
            co_await std::suspend_always();
        if (card->type() == CardType::Climax) {
            canceled = true;
            break;
        }
    }

    while (resZone->card(0))
        moveCard("res", 0, canceled ? "wr" : "clock");

    if (zone("clock")->count() >= 7)
        co_await levelUp();
}

int ServerPlayer::getMultiplierValue(const asn::Multiplier &m, const ServerCard *thisCard) {
    assert(m.type == asn::MultiplierType::ForEach);
    assert(m.forEach->type == asn::TargetType::SpecificCards);
    auto pzone = zone(asnZoneToString(m.zone));
    int cardCount = 0;
    for (int i = 0; i < pzone->count(); ++i) {
        auto card = pzone->card(i);
        if (!card)
            continue;

        const auto &tspec = m.forEach->targetSpecification;
        if (tspec->mode == asn::TargetMode::AllOther && card == thisCard)
            continue;

        if (checkCard(tspec->cards.cardSpecifiers, *card))
            cardCount++;
    }

    return cardCount;
}

asn::Ability TriggeredAbility::getAbility() const {
    if (type == ProtoCard && card.card)
        return card.card->abilities()[abilityId].ability;
    else if (type == ProtoGlobal)
        return globalAbility(static_cast<GlobalAbility>(abilityId));
    return {};
}
