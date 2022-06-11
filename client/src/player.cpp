#include "player.h"

#include <QVariant>
#include <QMetaObject>
#include <QQuickItem>
#include <QTimer>

#include "ability.pb.h"
#include "abilityCommands.pb.h"
#include "abilityEvents.pb.h"
#include "cardAttribute.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include "abilityUtils.h"
#include "cardDatabase.h"
#include "commonCardZone.h"
#include "deckView.h"
#include "deckList.h"
#include "game.h"
#include "hand.h"
#include "stage.h"

#include <QDebug>

#include <QQmlContext>

Player::Player(int id, Game *game, bool opponent)
    : mId(id), mGame(game), mOpponent(opponent) {
    auto hand = std::make_unique<Hand>(this, game);
    mHand = hand.get();
    mZones.emplace("hand", std::move(hand));
    auto wr = std::make_unique<CommonCardZone>(this, game, "wr");
    mZones.emplace("wr", std::move(wr));
    auto deck = std::make_unique<CommonCardZone>(this, game, "deck");
    mZones.emplace("deck", std::move(deck));
    auto clock = std::make_unique<CommonCardZone>(this, game, "clock");
    mZones.emplace("clock", std::move(clock));
    auto stock = std::make_unique<CommonCardZone>(this, game, "stock");
    mZones.emplace("stock", std::move(stock));
    auto stage= std::make_unique<Stage>(this, game);
    mStage = stage.get();
    mZones.emplace("stage", std::move(stage));
    auto level = std::make_unique<CommonCardZone>(this, game, "level");
    mZones.emplace("level", std::move(level));
    auto climax = std::make_unique<CommonCardZone>(this, game, "climax");
    mZones.emplace("climax", std::move(climax));
    auto memory = std::make_unique<CommonCardZone>(this, game, "memory");
    mZones.emplace("memory", std::move(memory));
    auto resolutionZone = std::make_unique<CommonCardZone>(this, game, "res");
    mZones.emplace("res", std::move(resolutionZone));
    mAbilityList = std::make_unique<ActivatedAbilities>(this, game);
    auto orderedView = std::make_unique<CommonCardZone>(this, game, "view");
    mZones.emplace("view", std::move(orderedView));

    if (!mOpponent) {
        auto deckView = std::make_unique<DeckView>(this, game);
        mDeckView = deckView.get();
        mZones.emplace("deckView", std::move(deckView));
    }
}

void Player::setDeck(const std::string &deck) {
    if (deck.empty())
        return;

    mDeckList.fromXml(deck);
    setDeckInternal();
}

void Player::setDeck(const DeckList &deck) {
    mDeckList = deck;
    setDeckInternal();
}

void Player::fillReferenceCache() {
    std::unordered_multimap<std::string, std::string> nameCodeCache;
    for (const auto &card: mDeckList.cards()) {
        auto info = CardDatabase::get().getCard(card.code);
        nameCodeCache.emplace(info->name(), info->code());
    }
    for (const auto &card: mDeckList.cards()) {
        auto info = CardDatabase::get().getCard(card.code);
        for (const auto &ref: info->references()) {
            if (nameCodeCache.contains(ref)) {
               auto range = nameCodeCache.equal_range(ref);
               for (auto it = range.first; it != range.second; ++it)
                   cardReferenceCache.emplace(card.code, it->second);
            }
        }
    }
}

void Player::setDeckInternal() {
    int cardCount = 0;
    for (const auto &card: mDeckList.cards())
        cardCount += card.count;

    auto deckZone = zone("deck");
    deckZone->model().addCards(cardCount, deckZone);
    mDeckSet = true;

    std::unordered_multimap<std::string, std::string> finalCache;
    fillReferenceCache();
}

Player* Player::getOpponent() const {
    if (mOpponent)
        return mGame->player();
    return mGame->opponent();
}

CardZone* Player::zone(std::string_view name) const {
    if (!mZones.count(name))
        return nullptr;

    return mZones.at(name).get();
}

CardZone *Player::zone(asn::Zone zone_) const {
    return zone(asnZoneToString(zone_));
}

void Player::processGameEvent(const std::shared_ptr<GameEvent> event) {
    if (event->event().Is<EventPlayerJoined>()) {
        assert(!mOpponent);
        EventPlayerJoined ev;
        event->event().UnpackTo(&ev);
        mGame->addOpponent(ev.player_info());
    } else if (event->event().Is<EventGameInfo>()) {
        EventGameInfo ev;
        event->event().UnpackTo(&ev);
        mGame->processGameInfo(ev.game_info());
    } else if (event->event().Is<EventDeckSet>()) {
        EventDeckSet ev;
        event->event().UnpackTo(&ev);
        mGame->setOpponentDeck(ev);
    } else if (event->event().Is<EventInitialHand>()) {
        EventInitialHand ev;
        event->event().UnpackTo(&ev);
        setInitialHand(ev);
    } else if (event->event().Is<EventMoveCard>()) {
        EventMoveCard ev;
        event->event().UnpackTo(&ev);
        moveCard(ev);
    } else if (event->event().Is<EventStartTurn>()) {
        startTurn();
    } else if (event->event().Is<EventClockPhase>()) {
        clockPhase();
    } else if (event->event().Is<EventMainPhase>()) {
        mainPhase();
    } else if (event->event().Is<EventPlayCard>()) {
        EventPlayCard ev;
        event->event().UnpackTo(&ev);
        playCard(ev);
    } else if (event->event().Is<EventSwitchStagePositions>()) {
        EventSwitchStagePositions ev;
        event->event().UnpackTo(&ev);
        switchStagePositions(ev);
    } else if (event->event().Is<EventClimaxPhase>()) {
        playClimax();
    } else if (event->event().Is<EventAttackDeclarationStep>()) {
        attackDeclarationStep();
    } else if (event->event().Is<EventDeclareAttack>()) {
        EventDeclareAttack ev;
        event->event().UnpackTo(&ev);
        declareAttack(ev);
    } else if (event->event().Is<EventSetCardAttr>()) {
        EventSetCardAttr ev;
        event->event().UnpackTo(&ev);
        setCardAttr(ev);
    } else if (event->event().Is<EventSetCardState>()) {
        EventSetCardState ev;
        event->event().UnpackTo(&ev);
        setCardState(ev);
    } else if (event->event().Is<EventCounterStep>()) {
        counterStep();
    } else if (event->event().Is<EventLevelUp>()) {
        levelUp();
    } else if (event->event().Is<EventClockToWr>()) {
        moveClockToWr();
    } else if (event->event().Is<EventEncoreStep>()) {
        encoreStep();
    } else if (event->event().Is<EventEndOfAttack>()) {
        endOfAttack();
    } else if (event->event().Is<EventRefresh>()) {
        refresh();
    } else if (event->event().Is<EventDiscardDownTo7>()) {
        discardTo7();
    } else if (event->event().Is<EventGameEnded>()) {
        EventGameEnded ev;
        event->event().UnpackTo(&ev);
        endGame(ev.victory());
    } else if (event->event().Is<EventAbilityActivated>()) {
        EventAbilityActivated ev;
        event->event().UnpackTo(&ev);
        activateAbilities(ev);
    } else if (event->event().Is<EventStartResolvingAbility>()) {
        EventStartResolvingAbility ev;
        event->event().UnpackTo(&ev);
        startResolvingAbility(ev);
    } else if (event->event().Is<EventEndResolvingAbilties>()) {
        endResolvingAbilties();
    } else if (event->event().Is<EventChooseCard>()) {
        EventChooseCard ev;
        event->event().UnpackTo(&ev);
        processChooseCard(ev);
    } else if (event->event().Is<EventAbilityResolved>()) {
        abilityResolved();
    } else if (event->event().Is<EventPlayAbility>()) {
        EventPlayAbility ev;
        event->event().UnpackTo(&ev);
        makeAbilityActive(ev);
    } else if (event->event().Is<EventMoveChoice>()) {
        EventMoveChoice ev;
        event->event().UnpackTo(&ev);
        processMoveChoice(ev);
    } else if (event->event().Is<EventMoveDestinationChoice>()) {
        EventMoveDestinationChoice ev;
        event->event().UnpackTo(&ev);
        processMoveDestinationChoice(ev);
    } else if (event->event().Is<EventMoveDestinationIndexChoice>()) {
        EventMoveDestinationIndexChoice ev;
        event->event().UnpackTo(&ev);
        processMoveDestinationIndexChoice(ev);
    } else if (event->event().Is<EventMoveTargetChoice>()) {
        EventMoveTargetChoice ev;
        event->event().UnpackTo(&ev);
        processMoveTargetChoice(ev);
    } else if (event->event().Is<EventDrawChoice>()) {
        EventDrawChoice ev;
        event->event().UnpackTo(&ev);
        processDrawChoice(ev);
    } else if (event->event().Is<EventPhaseEvent>()) {
        EventPhaseEvent ev;
        event->event().UnpackTo(&ev);
        mGame->setPhase(static_cast<asn::Phase>(ev.phase()));
    } else if (event->event().Is<EventRevealTopDeck>()) {
        EventRevealTopDeck ev;
        event->event().UnpackTo(&ev);
        revealTopDeck(ev);
    } else if (event->event().Is<EventConditionNotMet>()) {
        conditionNotMet();
    } else if (event->event().Is<EventPayCost>()) {
        payCostChoice();
    } else if (event->event().Is<EventSearchCard>()) {
        EventSearchCard ev;
        event->event().UnpackTo(&ev);
        processSearchCard(ev);
    } else if (event->event().Is<EventAbilityChoice>()) {
        EventAbilityChoice ev;
        event->event().UnpackTo(&ev);
        processAbilityChoice(ev);
    } else if (event->event().Is<EventAbilityGain>()) {
        EventAbilityGain ev;
        event->event().UnpackTo(&ev);
        processAbilityGain(ev);
    } else if (event->event().Is<EventRemoveAbility>()) {
        EventRemoveAbility ev;
        event->event().UnpackTo(&ev);
        processRemoveAbility(ev);
    } else if (event->event().Is<EventLookTopDeck>()) {
        EventLookTopDeck ev;
        event->event().UnpackTo(&ev);
        lookTopDeck(ev);
    } else if (event->event().Is<EventLook>()) {
        EventLook ev;
        event->event().UnpackTo(&ev);
        processLook(ev);
    } else if (event->event().Is<EventReveal>()) {
        EventReveal ev;
        event->event().UnpackTo(&ev);
        processReveal(ev);
    } else if (event->event().Is<EventSetCannotPlay>()) {
        EventSetCannotPlay ev;
        event->event().UnpackTo(&ev);
        setCannotPlay(ev);
    } else if (event->event().Is<EventEffectChoice>()) {
        EventEffectChoice ev;
        event->event().UnpackTo(&ev);
        processEffectChoice(ev);
    } else if (event->event().Is<EventSetPlayerAttr>()) {
        EventSetPlayerAttr ev;
        event->event().UnpackTo(&ev);
        setPlayerAttr(ev);
    } else if (event->event().Is<EventSetCardStateChoice>()) {
        EventSetCardStateChoice ev;
        event->event().UnpackTo(&ev);
        processSetCardStateChoice(ev);
    } else if (event->event().Is<EventSetCardStateTargetChoice>()) {
        EventSetCardStateTargetChoice ev;
        event->event().UnpackTo(&ev);
        processSetCardStateTargetChoice(ev);
    } else if (event->event().Is<EventSetCardBoolAttr>()) {
        EventSetCardBoolAttr ev;
        event->event().UnpackTo(&ev);
        processSetCardBoolAttr(ev);
    } else if (event->event().Is<EventRevealFromHand>()) {
        EventRevealFromHand ev;
        event->event().UnpackTo(&ev);
        processRevealFromHand(ev);
    } else if (event->event().Is<EventRuleActionChoice>()) {
        processRuleActionChoice();
    } else if (event->event().Is<EventPlayerLeft>()) {
        mGame->playerLeft();
    } else if (event->event().Is<EventPlayableCards>()) {
        EventPlayableCards ev;
        event->event().UnpackTo(&ev);
        processPlayableCards(ev);
    }
}

void Player::sendGameCommand(const google::protobuf::Message &command) {
    mGame->sendGameCommand(command, mId);
}

void Player::clockPhase() {
    mGame->setPhase(asn::Phase::ClockPhase);
    if (mOpponent)
        return;

    mGame->clockPhase();
    mHand->clockPhase();
}

void Player::mainPhase() {
    mGame->setPhase(asn::Phase::MainPhase);
    if (mOpponent)
        return;

    mGame->mainPhase();
    mHand->playTiming();
    mStage->mainPhase();
    highlightPlayableCards();
}

void Player::attackDeclarationStep() {
    mGame->setPhase(asn::Phase::AttackDeclarationStep);
    if (mOpponent) {
        //attackWithAll();
        return;
    }

    mGame->attackDeclarationStep();
    mStage->attackDeclarationStep();
}

void Player::declareAttack(const EventDeclareAttack &event) {
    if (event.stage_pos() >= 3)
        return;
    mGame->setPhase(asn::Phase::AttackPhase);
    mAttackingPos = event.stage_pos();
    mStage->attackDeclared(event.stage_pos());
    if (!isOpponent())
        mGame->attackDeclarationStepFinished();
}

void Player::sendAttackDeclaration(int pos, bool sideAttack) {
    CommandDeclareAttack cmd;
    cmd.set_stage_pos(pos);
    cmd.set_attack_type(sideAttack ? AttackType::SideAttack : FrontAttack);
    sendGameCommand(cmd);
}

void Player::startTurn() {
    mGame->startTurn(mOpponent);
}

void Player::mulliganFinished() {
    mHand->endMulligan();
    auto &cards = mHand->cards();
    CommandMulligan cmd;
    for (uint32_t i = 0; i < cards.size(); ++i) {
        if (cards[i].glow())
            cmd.add_ids(i);
    }

    sendGameCommand(cmd);
}

void Player::clockPhaseFinished() {
    mHand->endClockPhase();
    auto &cards = mHand->cards();
    CommandClockPhase cmd;
    for (uint32_t i = 0; i < cards.size(); ++i) {
        if (cards[i].selected()) {
            cmd.set_count(1);
            cmd.set_card_pos(i);
            break;
        }
    }

    sendGameCommand(cmd);
}

void Player::mainPhaseFinished() {
    mHand->endPlayTiming();
    mStage->endMainPhase();
    CommandClimaxPhase cmd;
    sendGameCommand(cmd);
}

void Player::counterStepFinished() {
    mHand->endPlayTiming();
    mGame->endCounterStep();
}

void Player::sendTakeDamageCommand() {
    counterStepFinished();
    sendGameCommand(CommandTakeDamage());
}

void Player::sendEncoreCommand() {
    mStage->endAttackPhase();
    sendGameCommand(CommandEncoreStep());
}

void Player::sendEndTurnCommand() {
    mStage->deactivateEncoreStep();
    sendGameCommand(CommandEndTurn());
}

void Player::setInitialHand(const EventInitialHand &event) {
    auto deck = zone("deck");
    if (!event.cards_size()) {
        for (int i = 0; i < event.count(); ++i) {
            deck->model().removeCard(deck->model().count() - 1);
            mHand->addCard();
        }
    } else {
        for (int i = 0; i < event.cards_size(); ++i) {
            deck->model().removeCard(deck->model().count() - 1);
            mHand->addCard(event.cards(i).id(), event.cards(i).code());
        }
    }

    if (mOpponent)
        return;

    QMetaObject::invokeMethod(mGame, "startMulligan", Q_ARG(QVariant, event.first_turn()));
    mHand->startMulligan();
}

void Player::createMovingCard(int id, const QString &code, const std::string &startZone, int startPos,
                              const std::string &targetZone, int targetPos, bool isUiAction,
                              bool dontFinishAction, bool noDelete) {
    if (isUiAction)
        mGame->startUiAction();
    else
        mGame->startAction();
    QString source = code;
    if (source.isEmpty())
        source = "cardback";
    QQmlComponent component(mGame->engine(), "qrc:/qml/MovingCard.qml");
    QQuickItem *obj = qobject_cast<QQuickItem*>(component.create(mGame->context()));
    obj->setParentItem(mGame->parentItem());
    obj->setProperty("uniqueId", id);
    obj->setProperty("code", source);
    obj->setProperty("opponent", mOpponent);
    obj->setProperty("isUiAction", isUiAction);
    obj->setProperty("dontFinishAction", dontFinishAction);
    obj->setProperty("noDelete", noDelete);
    obj->setProperty("mSource", source);
    obj->setProperty("startZone", QString::fromStdString(startZone));
    obj->setProperty("startPos", startPos);
    obj->setProperty("targetZone", QString::fromStdString(targetZone));
    obj->setProperty("targetPos", targetPos);
    obj->connect(obj, SIGNAL(moveFinished()), mGame, SLOT(cardMoveFinished()));
    QMetaObject::invokeMethod(obj, "startAnimation");
}

void Player::moveCard(const EventMoveCard &event) {
    auto startZoneName = event.start_zone() == "marker" ? "stage" : event.start_zone();
    auto targetZoneName = event.target_zone() == "marker" ? "stage" : event.target_zone();

    CardZone *startZone = zone(startZoneName);
    if (!startZone)
        return;

    CardZone *targetZone = zone(targetZoneName);
    if (!targetZone)
        return;

    auto &cards = startZone->cards();
    if (size_t(event.start_pos() + 1) > cards.size())
        return;

    QString code = QString::fromStdString(event.code());
    if (code.isEmpty()) {
        if (event.start_zone() == "marker") {
            auto &markers = cards[event.start_pos()].markers();
            if (!markers.empty())
                code = markers.back().qcode();
        } else {
            code = cards[event.start_pos()].qcode();
        }
    }

    bool dontFinishAction = false;
    if (event.target_zone() == "res") {
        mGame->pause(800);
        dontFinishAction = true;
    }

    int startPos = event.start_pos();
    auto startZoneStr = event.start_zone();
    auto deckView = zone("view");
    if (event.start_zone() == "deck" && deckView->model().count()) {
        auto deck = zone("deck");
        auto &cards = deckView->model().cards();
        for (size_t i = 0; i < cards.size(); ++i) {
            if (cards[i].id() == event.card_id()) {
                startPos = static_cast<int>(i);
                startZoneStr = "view";
                deck->model().removeCard(event.start_pos());
            }
        }
    }

    createMovingCard(event.card_id(), code, startZoneStr, startPos, event.target_zone(), event.target_pos(), false, dontFinishAction);
}

void Player::playCard(const EventPlayCard &event) {
    // player's chars are played before events from the server
    // TODO: play player's chars using EventMoveCard as the rest of the cards
    auto &cards = mHand->cards();
    if (size_t(event.hand_pos()) >= cards.size()
        || size_t(event.stage_pos()) >= mStage->cards().size())
        return;

    if (!mOpponent) {
        if (!mPlayingClimax)
            return;
        mPlayingClimax = false;
    }

    QString code = QString::fromStdString(event.code());
    if (code.isEmpty())
        code = cards[event.stage_pos()].qcode();

    auto cardInfo = CardDatabase::get().getCard(event.code());
    if (!cardInfo)
        return;

    std::string targetZone;
    if (cardInfo->type() == CardType::Climax)
        targetZone = "climax";
    else
        targetZone = "stage";

    createMovingCard(event.card_id(), code, "hand", event.hand_pos(), targetZone, event.stage_pos());
}

void Player::switchStagePositions(const EventSwitchStagePositions &event) {
    if (size_t(event.stage_pos_from()) >= mStage->cards().size()
        || size_t(event.stage_pos_to()) >= mStage->cards().size())
        return;

    mStage->swapCards(event.stage_pos_from(), event.stage_pos_to());
}

bool Player::canPlay(const Card &card) const {
    if (card.cannotPlay())
        return false;
    if (card.level() > mLevel)
        return false;
    if (card.cost() > static_cast<int>(zone("stock")->cards().size()))
        return false;
    if (!card.canPlayWoColorReq() &&
       (card.level() > 0 || card.type() == CardType::Climax)) {
        bool colorMatch = false;
        for (auto &clockCard: zone("clock")->cards()) {
            if (card.color() == clockCard.color()) {
                colorMatch = true;
                break;
            }
        }
        for (auto &levelCard: zone("level")->cards()) {
            if (card.color() == levelCard.color()) {
                colorMatch = true;
                break;
            }
        }
        if (!colorMatch)
            return false;
    }
    return true;
}

void Player::playClimax() {
    if (mOpponent)
        return;

    if (!mPlayingClimax) {
        sendGameCommand(CommandAttackPhase());
        return;
    }

    int cardPos = -1;
    for (int i = 0; i < mHand->cards().size(); ++i)
        if (mHand->cards()[i].id() == mClimaxId)
            cardPos = i;

    if (cardPos == -1) {
        sendGameCommand(CommandAttackPhase());
        return;
    }

    CommandPlayCard cmd;
    cmd.set_hand_pos(cardPos);
    sendGameCommand(cmd);
}

void Player::setCardAttr(const EventSetCardAttr &event) {
    auto pzone = zone(event.zone());
    if (!pzone)
        return;

    if (event.card_pos() >= pzone->model().count())
        return;

    if (event.zone() == "stage") {
        mStage->setAttr(event.card_pos(), event.attr(), event.value());
    } else {
        pzone->model().setAttr(event.card_pos(), event.attr(), event.value());
        if (event.zone() == "hand" && mHand->isPlayTiming())
            highlightPlayableCards();
    }
}

void Player::setCardState(const EventSetCardState &event) {
    if (event.stage_pos() >= 5)
        return;

    mStage->model().setState(event.stage_pos(), protoStateToState(event.state()));
}

void Player::counterStep() {
    mGame->setPhase(asn::Phase::CounterStep);
    if (mOpponent)
        return;

    mHand->playTiming();
    mGame->counterStep();
    auto &cards = mHand->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (canPlayCounter(cards[i]))
            mHand->model().setGlow(i, true);
    }
}

void Player::levelUp() {
    mLevel++;
    if  (mOpponent)
        return;

    mGame->showText("Level up", "(Choose a card in clock to send to level zone)");
    QMetaObject::invokeMethod(zone("clock")->visualItem(), "levelUp");
}

void Player::moveClockToWr() {
    auto clock = zone("clock");
    if (clock->cards().size() < 6)
        return;

    for (int i = 0; i < 6; ++i) {
        bool dontFinishAction = true;
        if (i == 5)
            dontFinishAction = false;
        auto &card = clock->cards()[0];
        createMovingCard(card.id(), card.qcode(), "clock", 0, "wr", -1, false, dontFinishAction);
    }
}

void Player::endOfAttack() {
    mStage->unhighlightAttacker();
}

void Player::encoreStep() {
    mGame->setPhase(asn::Phase::EncoreStep);
    if (mOpponent)
        return;

    mGame->encoreStep();
    mStage->encoreStep();
}

void Player::refresh() {
    auto wr = zone("wr");
    auto deck = zone("deck");
    for (int i = wr->model().count() - 1; i >= 0; --i) {
        wr->model().removeCard(i);
        deck->model().addCard(deck);
    }
}

void Player::discardTo7() {
    if (mOpponent)
        return;

    mGame->discardTo7();
    mHand->discardCard();
}

void Player::endGame(bool victory) {
    mGame->endGame(victory);
}

void Player::cardSelectedForLevelUp(int index) {
    mGame->hideText();

    CommandLevelUp cmd;
    cmd.set_clock_pos(index);
    sendGameCommand(cmd);
}

void Player::sendEncore(int pos) {
    mGame->pauseEncoreStep();
    mStage->deactivateEncoreStep();

    CommandEncoreCharacter cmd;
    cmd.set_stage_pos(pos);
    sendGameCommand(cmd);
}

void Player::sendDiscardCard(int id) {
    mGame->clearHelpText();
    mHand->deactivateDiscarding();

    CommandMoveCard cmd;
    cmd.set_start_pos(id);
    cmd.set_start_zone("hand");
    cmd.set_target_zone("wr");
    sendGameCommand(cmd);
}

void Player::sendPlayActAbility(int cardPos, int abilityId) {
    CommandPlayAct cmd;
    cmd.set_card_pos(cardPos);
    cmd.set_ability_id(abilityId);
    sendGameCommand(cmd);
}

void Player::cardPlayed(int handId, int stageId) {
    if (mHand->cards()[handId].type() == CardType::Climax) {
        // a little about playing climaxes
        // climax phase starts when player presses 'Next(to attack)' or when he plays a climax
        // this approach eliminates unnecessary button click 'Next(to climax phase)', but has a drawback
        // you have to commit to playing climax before 'on the start of climax phase' abilities resolve
        // 1. Player plays a climax, climax is not yet added to the climax zone.
        // 2. Player sends CommandClimaxPhase.
        // 3. At the start of the climax phase check timing.
        // 4. After the check timing player sends CommandPlayCard with this climax
        mHand->endPlayTiming();
        mStage->endMainPhase();

        mPlayingClimax = true;
        mClimaxId = mHand->cards()[handId].id();
        sendGameCommand(CommandClimaxPhase());
        return;
    }

    CommandPlayCard cmd;
    cmd.set_hand_pos(handId);
    cmd.set_stage_pos(stageId);
    sendGameCommand(cmd);
}

void Player::sendSwitchPositions(int from, int to) {
    CommandSwitchStagePositions cmd;
    cmd.set_stage_pos_from(from);
    cmd.set_stage_pos_to(to);
    sendGameCommand(cmd);
}

void Player::sendFromStageToWr(int pos) {
    auto &card = mStage->cards()[pos];
    for (const auto &marker: card.markers())
        createMovingCard(marker.id(), marker.qcode(), "stage", pos, "wr", -1, true, true, true);
    createMovingCard(card.id(), card.qcode(), "stage", pos, "wr", -1, true, false, true);
}

void Player::resetChoiceDialog() {
    auto ptr = mChoiceDialog.release();
    ptr->deleteLater();
}

std::vector<std::string> Player::cardReferences(const std::string &code) const {
    if (!cardReferenceCache.contains(code))
        return {};

    std::vector<std::string> references;
    auto range = cardReferenceCache.equal_range(code);
    for (auto it = range.first; it != range.second; ++it)
        references.push_back(it->second);

    return references;
}

void Player::sendPlayCounter(int handId) {
    counterStepFinished();

    CommandPlayCounter cmd;
    cmd.set_hand_pos(handId);
    sendGameCommand(cmd);
}

void Player::addCard(int id, QString code, QString zoneName, int targetPos) {
    auto pzone = zone(zoneName.toStdString());
    pzone->model().addCard(id, code.toStdString(), pzone, targetPos);
}

void Player::createMarkerView(int index) {
    if (mMarkerViews.contains(index))
        return;
    auto &cards = mStage->cards();
    if (static_cast<size_t>(index) > cards.size()) {
        qDebug() << "Wrong index during marker view creation";
        return;
    }
    if (cards[index].markers().empty())
        return;

    auto markerView = std::make_unique<CommonCardZone>(this, mGame, "view");
    for (const auto &marker: cards[index].markers())
        markerView->model().addCard(marker.id(), marker.code(), mStage);
    connect(markerView->visualItem(), SIGNAL(deleteMarkerView(int)), this, SLOT(deleteMarkerView(int)));

    auto [x, y] = mStage->coords(index);
    QMetaObject::invokeMethod(markerView->visualItem(), "positionMarkerView", Q_ARG(QVariant, x), Q_ARG(QVariant, y));
    markerView->visualItem()->setProperty("mViewMode", Game::MarkerMode);
    markerView->visualItem()->setProperty("markerStagePos", index);
    mMarkerViews.emplace(index, std::move(markerView));
}

void Player::deleteMarkerView(int index) {
    if (!mMarkerViews.contains(index))
        return;
    auto &markerView = mMarkerViews.at(index);
    markerView->visualItem()->deleteLater();
    mMarkerViews.erase(index);
}

void Player::testAction()
{
    sendGameCommand(CommandTest());

    //QTimer::singleShot(1000, this, [this]() { createMovingCard(1, "IMC/W43-046", "deck", 0, "level", 0); });
    //QMetaObject::invokeMethod(zone("view")->visualItem(), "getCardOrder");

    //createMovingCard("IMC/W43-046", "hand", 1, "wr", 0, true, false, true);
    //QMetaObject::invokeMethod(zone("stage")->visualItem(), "getCardPos", Q_ARG(QVariant, 1));
}

bool Player::playCards(CardModel &hand) {
    auto &cards = mStage->cards();
    for (int i = 0; i < 5; ++i) {
        if (cards[i].cardPresent())
            continue;
        auto &handCards = hand.cards();
        for (int j = 0; (size_t)j < handCards.size(); ++j) {
            if (canPlay(handCards[j]) && handCards[j].type() != CardType::Climax) {
                CommandPlayCard cmd;
                cmd.set_hand_pos(j);
                cmd.set_stage_pos(i);
                sendGameCommand(cmd);
                return false;
            }
        }
    }

    return true;
}

void Player::attackWithAll() {
    auto &cards = mStage->cards();
    for (int i = 0; i < 3; ++i) {
        if (cards[i].cardPresent() && cards[i].state() == asn::State::Standing) {
            CommandDeclareAttack cmd;
            cmd.set_attack_type(AttackType::FrontAttack);
            cmd.set_stage_pos(i);
            sendGameCommand(cmd);
            return;
        }
    }
}
