#include "serverPlayer.h"

#include <algorithm>

#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include "cardDatabase.h"
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
        mulligan(mulliganCmd);
    } else if (cmd.command().Is<CommandClockPhase>()) {
        CommandClockPhase clockCmd;
        cmd.command().UnpackTo(&clockCmd);
        processClockPhaseResult(clockCmd);
    } else if (cmd.command().Is<CommandPlayCard>()) {
        CommandPlayCard playCmd;
        cmd.command().UnpackTo(&playCmd);
        playCard(playCmd);
    } else if (cmd.command().Is<CommandSwitchStagePositions>()) {
        CommandSwitchStagePositions switchCmd;
        cmd.command().UnpackTo(&switchCmd);
        switchPositions(switchCmd);
    } else if (cmd.command().Is<CommandClimaxPhase>()) {
        climaxPhase();
    } else if (cmd.command().Is<CommandAttackPhase>()) {
        attackDeclarationStep();
    } else if (cmd.command().Is<CommandDeclareAttack>()) {
        CommandDeclareAttack declareAttackCmd;
        cmd.command().UnpackTo(&declareAttackCmd);
        declareAttack(declareAttackCmd);
    } else if (cmd.command().Is<CommandTakeDamage>()) {
        damageStep();
    } else if (cmd.command().Is<CommandLevelUp>()) {
        CommandLevelUp levelUpCmd;
        cmd.command().UnpackTo(&levelUpCmd);
        performLevelUp(levelUpCmd);
    } else if (cmd.command().Is<CommandEncoreCharacter>()) {
        CommandEncoreCharacter encoreCmd;
        cmd.command().UnpackTo(&encoreCmd);
        encoreCharacter(encoreCmd);
    } else if (cmd.command().Is<CommandEncoreStep>()) {
        mGame->startAsyncTask(mGame->encoreStep());
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

void ServerPlayer::startTurn() {
    sendToBoth(EventStartTurn());

    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (card && card->state() != StateStanding)
            setCardState(card, StateStanding);
    }

    drawCards(1);

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
        auto code = eventPrivate.add_codes();
        *code = card->code();
        hand->addCard(std::move(card));
    }

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);
}

void ServerPlayer::mulligan(const CommandMulligan &cmd) {
    if (cmd.ids_size()) {
        std::vector<int> ids;
        for (int i = 0; i < cmd.ids_size(); ++i)
            ids.push_back(cmd.ids(i));

        moveCards("hand", ids, "wr");
        drawCards(static_cast<int>(ids.size()));
    }
    mMulliganFinished = true;
    mGame->endMulligan();
}

void ServerPlayer::drawCards(int number) {
    auto deck = zone("deck");

    for (int i = 0; i < number; ++i) {
        moveCard("deck", deck->count() - 1, "hand");
    }
}

void ServerPlayer::moveCards(std::string_view startZoneName,  const std::vector<int> &cardIds, std::string_view targetZoneName) {
    auto sortedIds = cardIds;
    std::sort(sortedIds.begin(), sortedIds.end());
    for (int i = static_cast<int>(sortedIds.size()); i-- > 0;)
        moveCard(startZoneName, sortedIds[i], targetZoneName);
}

bool ServerPlayer::moveCard(std::string_view startZoneName, int id, std::string_view targetZoneName) {
    ServerCardZone *startZone = zone(startZoneName);
    if (!startZone)
        return false;

    ServerCardZone *targetZone = zone(targetZoneName);
    if (!targetZone)
        return false;

    auto cardPtr = startZone->takeCard(id);
    if (!cardPtr)
        return false;

    ServerCard *card = targetZone->addCard(std::move(cardPtr));

    if (startZoneName == "stage" && startZoneName != "stage")
        card->reset();

    EventMoveCard eventPublic;
    eventPublic.set_id(static_cast<google::protobuf::uint32>(id));
    eventPublic.set_startzone(startZone->name());
    eventPublic.set_targetzone(targetZone->name());

    if (startZone->type() == ZoneType::HiddenZone && targetZone->type() == ZoneType::PublicZone)
        eventPublic.set_code(card->code());

    EventMoveCard eventPrivate(eventPublic);

    if (startZone->type() == ZoneType::HiddenZone && targetZone->type() == ZoneType::PrivateZone)
        eventPrivate.set_code(card->code());

    // revealing card from hand, opponent didn't see this card yet
    if (startZone->type() == ZoneType::PrivateZone && targetZone->type() == ZoneType::PublicZone)
        eventPublic.set_code(card->code());

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    return true;
}

void ServerPlayer::moveTopDeck(std::string_view targetZoneName) {
    auto deck = zone("deck");
    moveCard("deck", deck->count() - 1, targetZoneName);
    // check for refresh
}

void ServerPlayer::processClockPhaseResult(const CommandClockPhase &cmd) {
    if (cmd.count()) {
        moveCard("hand", cmd.cardid(), "clock");
        drawCards(2);
    }

    sendToBoth(EventMainPhase());

    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::GetDescriptor()->name());
    addExpectedCommand(CommandClimaxPhase::GetDescriptor()->name());
    addExpectedCommand(CommandAttackPhase::GetDescriptor()->name());
    addExpectedCommand(CommandSwitchStagePositions::GetDescriptor()->name());
    //TODO activate ability
}

void ServerPlayer::playCard(const CommandPlayCard &cmd) {
    auto *hand = zone("hand");
    auto cardPtr = hand->card(cmd.handid());
    if (!cardPtr)
        return;

    if (cardPtr->type() == CardType::Character)
        playCharacter(cmd);
    else if (cardPtr->type() == CardType::Climax)
        playClimax(cmd.handid());
}

void ServerPlayer::playCharacter(const CommandPlayCard &cmd) {
    ServerCardZone *hand = zone("hand");
    ServerCardZone *stage = zone("stage");
    if (cmd.stageid() >= stage->count())
        return;

    auto cardPtr = hand->card(cmd.handid());
    if (!cardPtr)
        return;

    if (!canPlay(cardPtr))
        return;

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

    if (oldStageCard)
        zone("wr")->addCard(std::move(oldStageCard));

    auto stock = zone("stock");
    for (int i = 0; i < cardInPlay->cost(); ++i)
        moveCard("stock", stock->count() - 1, "wr");
}

void ServerPlayer::playClimax(int handIndex) {
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
    if (mGame->phase() == Phase::Climax && card->type() != CardType::Climax)
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
    mGame->setPhase(Phase::Climax);
    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::GetDescriptor()->name());
    sendToBoth(EventClimaxPhase());
}

void ServerPlayer::endOfAttack() {
    setAttackingCard(nullptr);
    sendToBoth(EventEndOfAttack());
    attackDeclarationStep();
}

void ServerPlayer::attackDeclarationStep() {
    clearExpectedComands();

    bool canAttack = false;
    auto stage = zone("stage");
    for (int i = 0; i < 3; ++i) {
        if (stage->card(i) && stage->card(i)->state() == StateStanding) {
            canAttack = true;
            break;
        }
    }

    if (canAttack) {
        mGame->setPhase(Phase::AttackDeclarationStep);
        addExpectedCommand(CommandDeclareAttack::GetDescriptor()->name());
        addExpectedCommand(CommandEncoreStep::GetDescriptor()->name());
        sendToBoth(EventAttackDeclarationStep());
    } else {
        mGame->startAsyncTask(mGame->encoreStep());
    }
}

bool isCenterStagePosition(int pos) { return pos >= 3 ? false : true; }
void ServerPlayer::declareAttack(const CommandDeclareAttack &cmd) {
    auto stage = zone("stage");
    if (!isCenterStagePosition(cmd.stageid()))
        return;
    auto attCard = stage->card(cmd.stageid());
    if (attCard->state() != StateStanding)
        return;

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
        addAttributeBuff(AttrSoul, cmd.stageid(), 1);
    else if (type == AttackType::SideAttack)
        addAttributeBuff(AttrSoul, cmd.stageid(), -battleOpp->level());

    triggerStep(cmd.stageid());
}

void ServerPlayer::addAttributeBuff(CardAttribute attr, int pos, int delta, int duration) {
    auto stage = zone("stage");
    auto card = stage->card(pos);
    if (!card)
        return;

    card->addAttrBuff(attr, delta, duration);

    EventSetCardAttr event;
    event.set_stageid(pos);
    event.set_attr(attr);
    event.set_value(attr == AttrSoul ? card->soul() : card->power());
    sendToBoth(event);
}

void ServerPlayer::triggerStep(int pos) {
    moveTopDeck("res");
    auto card = zone("res")->card(0);
    for (auto trigger: card->triggers()) {
        switch(trigger) {
        case Trigger::Soul:
            addAttributeBuff(AttrSoul, pos, 1);
            break;
        }
    }
    moveCard("res", 0, "stock");

    mGame->opponentOfPlayer(mId)->counterStep();
}

void ServerPlayer::counterStep() {
    addExpectedCommand(CommandTakeDamage::GetDescriptor()->name());
    sendToBoth(EventCounterStep());
}

void ServerPlayer::damageStep() {
    mGame->setPhase(Phase::DamageStep);
    clearExpectedComands();
    auto attCard = mGame->opponentOfPlayer(mId)->attackingCard();
    if (!attCard)
        return;

    auto resZone = zone("res");
    bool canceled = false;
    for (int i = 0; i < attCard->soul(); ++i) {
        moveTopDeck("res");
        auto card = resZone->topCard();
        assert(card);
        if (card->type() == CardType::Climax) {
            canceled = true;
            break;
        }
    }

    while (resZone->card(0))
        moveCard("res", 0, canceled ? "wr" : "clock");

    if (zone("clock")->count() >= 7) {
        levelUp();
        return;
    }

    mGame->battleStep();
}

void ServerPlayer::levelUp() {
    clearExpectedComands();
    addExpectedCommand(CommandLevelUp::GetDescriptor()->name());
    sendToBoth(EventLevelUp());
}

void ServerPlayer::performLevelUp(const CommandLevelUp& cmd) {
    if (cmd.clockid() > 6)
        return;

    auto clock = zone("clock");
    if (clock->count() < 7)
        return;

    if (!moveCard("clock", cmd.clockid(), "level"))
        return;

    clearExpectedComands();

    mLevel++;
    auto wr = zone("wr");
    for (int i = 0; i < 6; ++i) {
        auto card = clock->takeCard(0);
        wr->addCard(std::move(card));
    }

    sendToBoth(EventClockToWr());

    if (mGame->phase() == Phase::DamageStep) {
        mGame->battleStep();
    }
}

Resumable ServerPlayer::encoreStep() {
    clearExpectedComands();

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
                encoreCharacter(encoreCmd);
                if (!needEncore())
                    break;
            } else if (cmd.command().Is<CommandEndTurn>()) {
                for (int i = 0; i < 5; ++i) {
                    auto card = stage->card(i);
                    if (card && card->state() == StateReversed) {
                        moveCard("stage", i, "wr");
                    }
                }
                break;
            }
        }

        clearExpectedComands();
    }
}

void ServerPlayer::encoreCharacter(const CommandEncoreCharacter &cmd) {
    if (cmd.stageid() < 0 || cmd.stageid() >= 5)
        return;

    auto stage = zone("stage");
    if (!stage->card(cmd.stageid())
        || stage->card(cmd.stageid())->state() != StateReversed)
        return;

    moveCard("stage", cmd.stageid(), "wr");

    // ...
}

void ServerPlayer::endPhase() {
    if (mActive) {
        //discard to 7
    }

    auto climax = zone("climax");
    if (climax->count() > 0)
        moveCard("climax", 0, "wr");

    endOfTurnEffectValidation();
}

void ServerPlayer::setCardState(ServerCard *card, CardState state) {
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
            event.set_attr(AttrPower);
            event.set_value(card->power());
            sendToBoth(event);
        }
        if (oldSoul != card->soul()) {
            EventSetCardAttr event;
            event.set_stageid(i);
            event.set_attr(AttrSoul);
            event.set_value(card->soul());
            sendToBoth(event);
        }
    }
}

ServerCard *ServerPlayer::battleOpponent(ServerCard *card) const {
    auto opponent = mGame->opponentOfPlayer(mId);
    if (!opponent)
        return nullptr;

    return opponent->zone("stage")->card(card->pos());
}
