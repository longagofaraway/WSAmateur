#include "serverPlayer.h"

#include <algorithm>

#include "abilities.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include <abilities.h>

#include "abilityUtils.h"
#include "cardDatabase.h"
#include "globalAbilities/globalAbilities.h"
#include "serverCardZone.h"
#include "serverGame.h"
#include "serverProtocolHandler.h"
#include "serverStage.h"

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
    } else if (cmd.command().Is<CommandSwitchStagePositions>()) {
        CommandSwitchStagePositions switchCmd;
        cmd.command().UnpackTo(&switchCmd);
        switchPositions(switchCmd);
    } else if (cmd.command().Is<CommandClimaxPhase>()) {
        climaxPhase();
    } else if (cmd.command().Is<CommandAttackPhase>()) {
        startAttackPhase();
    } else if (cmd.command().Is<CommandDeclareAttack>()) {
        CommandDeclareAttack declareAttackCmd;
        cmd.command().UnpackTo(&declareAttackCmd);
        mGame->startAsyncTask(declareAttack(declareAttackCmd));
    } else if (cmd.command().Is<CommandTakeDamage>()) {
        mGame->startAsyncTask(mGame->continueFromDamageStep());
    } else if (cmd.command().Is<CommandEncoreStep>()) {
        mGame->startAsyncTask(mGame->encoreStep());
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
    mExpectedCommands.push_back(CommandReadyToStart::GetDescriptor()->name());
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
    mExpectedCommands.emplace_back(CommandMulligan::GetDescriptor()->name(), 1);
}

Resumable ServerPlayer::startTurn() {
    sendToBoth(EventStartTurn());

    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (card && card->state() != StateStanding)
            setCardState(card, StateStanding);
    }

    drawCards(1);
    co_await mGame->checkTiming();

    sendToBoth(EventClockPhase());

    addExpectedCommand(CommandClockPhase::GetDescriptor()->name());
}

void ServerPlayer::addExpectedCommand(const std::string &command) {
    mExpectedCommands.push_back(command);
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

    auto cardPtr = startZone->takeCard(startId);
    if (!cardPtr)
        return false;

    ServerCard *card = targetZone->addCard(std::move(cardPtr));

    if (startZoneName == "stage" && targetZoneName != "stage")
        card->reset();

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

    if (startZoneName == "stage" && targetZoneName != "stage")
        validateContAbilitiesOnStageChanges();

    checkZoneChangeTrigger(card, startZoneName, targetZoneName);
    if (enableGlobEncore)
        checkGlobalEncore(card, targetZone->count() - 1, startZoneName, targetZoneName);

    return true;
}

bool ServerPlayer::moveCardToStage(ServerCardZone *startZone, int startId, ServerCardZone *targetZone, int targetId) {
    if (targetId >= 5)
        return false;
    auto card = startZone->takeCard(startId);
    if (!card)
        return false;

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
    if (oldStageCard)
        pOldStageCard = zone("wr")->addCard(std::move(oldStageCard));

    validateContAbilitiesOnStageChanges();
    activateContAbilities(cardOnStage);
    checkZoneChangeTrigger(cardOnStage, startZone->name(), "stage");
    if (pOldStageCard)
        checkZoneChangeTrigger(pOldStageCard, "stage", "wr");

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
    addExpectedCommand(CommandPlayCard::GetDescriptor()->name());
    addExpectedCommand(CommandClimaxPhase::GetDescriptor()->name());
    addExpectedCommand(CommandAttackPhase::GetDescriptor()->name());
    addExpectedCommand(CommandSwitchStagePositions::GetDescriptor()->name());
    //TODO activate ability
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
    if (oldStageCard)
        pOldStageCard = zone("wr")->addCard(std::move(oldStageCard));

    auto stock = zone("stock");
    for (int i = 0; i < cardInPlay->cost(); ++i)
        moveCard("stock", stock->count() - 1, "wr");
    validateContAbilitiesOnStageChanges();
    activateContAbilities(cardInPlay);
    checkZoneChangeTrigger(cardInPlay, "hand", "stage");
    if (pOldStageCard)
        checkZoneChangeTrigger(pOldStageCard, "stage", "wr");
    co_await mGame->checkTiming();
}

Resumable ServerPlayer::playClimax(int handIndex) {
    auto hand = zone("hand");
    auto climaxZone = zone("climax");

    auto card = hand->takeCard(handIndex);
    climaxZone->addCard(std::move(card));
    auto cardPtr = climaxZone->card(0);

    EventPlayCard eventPublic;
    eventPublic.set_handid(static_cast<google::protobuf::uint32>(handIndex));
    EventPlayCard eventPrivate(eventPublic);
    eventPublic.set_code(cardPtr->code());

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    checkZoneChangeTrigger(cardPtr, "hand", "climax");
    co_await mGame->checkTiming();

    //process climax effects

    attackDeclarationStep();
}

void ServerPlayer::switchPositions(const CommandSwitchStagePositions &cmd) {
    ServerCardZone *stage = zone("stage");
    if (cmd.stageidfrom() >= stage->count()
        || cmd.stageidto() >= stage->count())
        return;

    stage->switchPositions(cmd.stageidfrom(), cmd.stageidto());
    EventSwitchStagePositions event;
    event.set_stageidfrom(cmd.stageidfrom());
    event.set_stageidto(cmd.stageidto());

    sendToBoth(event);
}

bool ServerPlayer::canPlay(ServerCard *card) {
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
    addExpectedCommand(CommandPlayCard::GetDescriptor()->name());
    sendToBoth(EventClimaxPhase());
}

void ServerPlayer::endOfAttack() {
    setAttackingCard(nullptr);
    sendToBoth(EventEndOfAttack());
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

void ServerPlayer::startAttackPhase() {
    if (canAttack())
        attackDeclarationStep();
    else
        mGame->startAsyncTask(mGame->encoreStep());
}

void ServerPlayer::attackDeclarationStep() {
    mGame->setPhase(ServerPhase::AttackDeclarationStep);
    addExpectedCommand(CommandDeclareAttack::GetDescriptor()->name());
    addExpectedCommand(CommandEncoreStep::GetDescriptor()->name());
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
    auto battleOpp = battleOpponent(attCard);
    if (!battleOpp)
        type = AttackType::DirectAttack;

    setAttackType(type);
    EventDeclareAttack event;
    event.set_stageid(cmd.stageid());
    event.set_attacktype(type);
    sendToBoth(event);

    if (type == AttackType::DirectAttack)
        addAttributeBuff(asn::AttributeType::Soul, cmd.stageid(), 1);
    else if (type == AttackType::SideAttack)
        addAttributeBuff(asn::AttributeType::Soul, cmd.stageid(), -battleOpp->level());

    checkOnAttack(attCard);
    co_await mGame->checkTiming();

    co_await triggerStep(cmd.stageid());
}

void ServerPlayer::addAttributeBuff(asn::AttributeType attr, int pos, int delta, int duration) {
    auto stage = zone("stage");
    auto card = stage->card(pos);
    if (!card)
        return;

    card->addAttrBuff(attr, delta, duration);

    EventSetCardAttr event;
    event.set_stageid(pos);
    event.set_attr(attrTypeToProto(attr));
    event.set_value(attr == asn::AttributeType::Soul ? card->soul() : card->power());
    sendToBoth(event);
}

void ServerPlayer::changeAttribute(ServerCard *card, asn::AttributeType attr, int delta) {
    card->changeAttr(attr, delta);

    EventSetCardAttr event;
    event.set_stageid(card->pos());
    event.set_attr(attrTypeToProto(attr));
    event.set_value(attr == asn::AttributeType::Soul ? card->soul() : card->power());
    sendToBoth(event);
}

Resumable ServerPlayer::triggerStep(int pos) {
    sendPhaseEvent(asn::Phase::TriggerStep);
    moveTopDeck("res");
    auto card = zone("res")->card(0);
    // end of game
    if (!card)
        co_await std::suspend_always();
    for (auto trigger: card->triggers()) {
        switch(trigger) {
        case TriggerIcon::Soul:
            addAttributeBuff(asn::AttributeType::Soul, pos, 1);
            break;
        case TriggerIcon::Door:
        case TriggerIcon::Choice:
        case TriggerIcon::Wind:
        case TriggerIcon::Treasure:
        case TriggerIcon::Bag:
        case TriggerIcon::Book:
            EventAbilityActivated event;
            auto ab = event.add_abilities();
            ab->set_zone("res");
            ab->set_type(ProtoAbilityType::ProtoClimaxTrigger);
            ab->set_cardid(0);
            ab->set_abilityid(static_cast<::google::protobuf::int32>(trigger));
            ab->set_cardcode(card->code());
            auto uniqueId = abilityHash(*ab);
            ab->set_uniqueid(uniqueId);
            sendToBoth(event);

            EventStartResolvingAbility evStart;
            evStart.set_uniqueid(uniqueId);
            sendToBoth(evStart);

            mContext = AbilityContext();
            mContext.thisCard = CardImprint("res", 0, card);
            co_await playAbility(triggerAbility(trigger));

            EventAbilityResolved ev2;
            ev2.set_uniqueid(uniqueId);
            sendToBoth(ev2);

            sendToBoth(EventEndResolvingAbilties());
            break;
        }
    }
    moveCard("res", 0, "stock");

    co_await mGame->checkTiming();

    if (attackType() == FrontAttack)
        mGame->opponentOfPlayer(mId)->counterStep();
    else
        co_await mGame->continueFromDamageStep();
}

void ServerPlayer::counterStep() {
    addExpectedCommand(CommandTakeDamage::GetDescriptor()->name());
    sendToBoth(EventCounterStep());
}

Resumable ServerPlayer::damageStep() {
    mGame->setPhase(ServerPhase::DamageStep);

    clearExpectedComands();
    auto attCard = mGame->opponentOfPlayer(mId)->attackingCard();
    if (!attCard)
        co_return;

    if (attCard->zone()->name() != "stage")
        co_return;

    auto resZone = zone("res");
    bool canceled = false;
    for (int i = 0; i < attCard->soul(); ++i) {
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

    co_await mGame->checkTiming();
}

Resumable ServerPlayer::levelUp() {
    clearExpectedComands();

    if (zone("level")->count() == 3) {
        sendEndGame(false);
        co_await std::suspend_always();
    }
    addExpectedCommand(CommandLevelUp::GetDescriptor()->name());
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
        addExpectedCommand(CommandEncoreCharacter::GetDescriptor()->name());
        addExpectedCommand(CommandEndTurn::GetDescriptor()->name());

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
                        moveCard("stage", i, "wr", false, false);
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
            addExpectedCommand(CommandMoveCard::GetDescriptor()->name());
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
    auto deck = zone("deck");
    auto wr = zone("wr");
    int count = wr->count();
    if (!count) {
        sendEndGame(false);
        return;
    }
    for (int i = 0; i < count; ++i) {
        auto card = wr->takeTopCard();
        deck->addCard(std::move(card));
    }
    deck->shuffle();
    sendToBoth(EventRefresh());

    TriggeredAbility a;
    a.type = ProtoRuleAction;
    a.abilityId = static_cast<int>(RuleAction::RefreshPoint);
    mQueue.push_back(a);
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

void ServerPlayer::setCardState(ServerCard *card, CardState state) {
    if (card->state() == state)
        return;
    card->setState(state);

    EventSetCardState event;
    event.set_stageid(card->pos());
    event.set_state(state);
    sendToBoth(event);
}

void ServerPlayer::endOfTurnEffectValidation() {
    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;
        int oldPower = card->power();
        int oldSoul = card->soul();
        card->validateBuffs();
        if (oldPower != card->power()) {
            EventSetCardAttr event;
            event.set_stageid(i);
            event.set_attr(ProtoAttrPower);
            event.set_value(card->power());
            sendToBoth(event);
        }
        if (oldSoul != card->soul()) {
            EventSetCardAttr event;
            event.set_stageid(i);
            event.set_attr(ProtoAttrSoul);
            event.set_value(card->soul());
            sendToBoth(event);
        }
        auto &abs = card->abilities();
        auto it = abs.rbegin();
        while (it != abs.rend()) {
            if (it->permanent)
                break;
            if (--it->duration != 0) {
                ++it;
                continue;
            }

            EventRemoveAbility event;
            event.set_cardid(card->pos());
            event.set_zone(card->zone()->name());
            event.set_abilityid(std::distance(abs.begin(), (it+1).base()));
            sendToBoth(event);

            it = std::reverse_iterator(abs.erase((++it).base()));
        }
    }
}

ServerCard *ServerPlayer::battleOpponent(ServerCard *card) const {
    auto opponent = mGame->opponentOfPlayer(mId);
    if (!opponent)
        return nullptr;

    return opponent->zone("stage")->card(card->pos());
}

asn::Ability TriggeredAbility::getAbility() const {
    if (type == ProtoCard && card.card)
        return card.card->abilities()[abilityId].ability;
    else if (type == ProtoGlobal)
        return globalAbility(static_cast<GlobalAbility>(abilityId));
    return {};
}
