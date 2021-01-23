#include "player.h"

#include <QVariant>
#include <QMetaObject>
#include <QQuickItem>

#include "cardAttribute.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include "cardDatabase.h"
#include "commonCardZone.h"
#include "deck.h"
#include "game.h"
#include "hand.h"
#include "stage.h"
#include "waitingRoom.h"

#include <QDebug>

Player::Player(int id, Game *game, bool opponent)
    : mId(id), mGame(game), mOpponent(opponent) {
    auto hand = std::make_unique<Hand>(this, game);
    mHand = hand.get();
    mZones.emplace("hand", std::move(hand));
    auto wr = std::make_unique<WaitingRoom>(this, game);
    /*wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));
    wr->model().addCard(std::string("IMC/W43-009"));*/
    mZones.emplace("wr", std::move(wr));
    auto deck = std::make_unique<Deck>(this, game);
    mZones.emplace("deck", std::move(deck));
    auto clock = std::make_unique<CommonCardZone>(this, game, "Clock");
    mZones.emplace("clock", std::move(clock));
    auto stock = std::make_unique<CommonCardZone>(this, game, "Stock");
    mZones.emplace("stock", std::move(stock));
    auto stage= std::make_unique<Stage>(this, game);
    mStage = stage.get();
    mZones.emplace("stage", std::move(stage));
    auto level = std::make_unique<CommonCardZone>(this, game, "Level");
    mZones.emplace("level", std::move(level));
    auto climax = std::make_unique<CommonCardZone>(this, game, "Climax");
    mZones.emplace("climax", std::move(climax));
    auto resolutionZone = std::make_unique<CommonCardZone>(this, game, "ResolutionZone");
    mZones.emplace("res", std::move(resolutionZone));
}

CardZone* Player::zone(std::string_view name) const {
    if (!mZones.count(name))
        return nullptr;

    return mZones.at(name).get();
}

void Player::processGameEvent(const std::shared_ptr<GameEvent> event) {
    if (event->event().Is<EventInitialHand>()) {
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
    }
}

void Player::sendGameCommand(const google::protobuf::Message &command) {
    mGame->sendGameCommand(command, mId);
}

void Player::clockPhase() {
    if (mOpponent)
        return;

    mGame->clockPhase();
    mHand->clockPhase();
}

void Player::mainPhase() {
    if (mOpponent)
        return;

    mGame->mainPhase();
    mHand->mainPhase();
    mStage->mainPhase();
    auto &cards = mHand->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (canPlay(cards[i]))
            mHand->model().setGlow(i, true);
    }
}

void Player::attackDeclarationStep() {
    if (mOpponent) {
        attackWithAll();
        return;
    }

    mGame->attackDeclarationStep();
    mStage->attackDeclarationStep();
}

void Player::declareAttack(const EventDeclareAttack &event) {
    if (event.stageid() >= 3)
        return;
    mStage->attackDeclared(event.stageid());
    if (!isOpponent())
        mGame->attackDeclarationStepFinished();
}

void Player::sendAttackDeclaration(int pos, bool sideAttack) {
    CommandDeclareAttack cmd;
    cmd.set_stageid(pos);
    cmd.set_attacktype(sideAttack ? AttackType::SideAttack : FrontAttack);
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
            cmd.set_cardid(i);
        }
    }

    sendGameCommand(cmd);
}

void Player::mainPhaseFinished() {
    mHand->endMainPhase();
    mStage->endMainPhase();
    CommandAttackPhase cmd;
    sendGameCommand(cmd);
}

void Player::sendClimaxPhaseCommand() {
    // a little about playing climaxes
    // climax phase starts when player presses 'Next(to attack)' or when he plays a climax
    // this approach eliminates unnecessary button click 'Next(to climax phase)', but has a drawback in current realization
    // 1. Player plays a climax, climax is added to the climax zone and removed from hand on player's side,
    //    but not on the server's. Climax effects are not performed yet. Player sends CommandClimaxPhase.
    // 2. Server triggers 'At the start of climax phase' abilities. Player resolves abilities.
    // 3. Player sends 'play climax' command. Climax effects are resolved.
    // If there are 'At the start of climax phase' abilities that mess with card count in hand or
    // smth like that we are fcked;
    mHand->endMainPhase();
    mStage->endMainPhase();
    sendGameCommand(CommandClimaxPhase());
}

void Player::sendTakeDamageCommand() {
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
    if (!event.codes_size()) {
        for (int i = 0; i < event.count(); ++i)
            mHand->addCard();
    } else {
        for (int i = 0; i < event.codes_size(); ++i)
            mHand->addCard(event.codes(i));
    }

    if (mOpponent)
        return;

    QMetaObject::invokeMethod(mGame, "startMulligan", Q_ARG(QVariant, event.firstturn()));
    mHand->startMulligan();
}

void Player::createMovingCard(const QString &code, const std::string &startZone, int startId,
                              const std::string &targetZone, int targetId, bool isUiAction,
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
    obj->setProperty("code", code);
    obj->setProperty("opponent", mOpponent);
    obj->setProperty("isUiAction", isUiAction);
    obj->setProperty("dontFinishAction", dontFinishAction);
    obj->setProperty("noDelete", noDelete);
    obj->setProperty("mSource", source);
    obj->setProperty("startZone", QString::fromStdString(startZone));
    obj->setProperty("startId", startId);
    obj->setProperty("targetZone", QString::fromStdString(targetZone));
    obj->setProperty("targetId", targetId);
    obj->connect(obj, SIGNAL(moveFinished()), mGame, SLOT(cardMoveFinished()));
    QMetaObject::invokeMethod(obj, "startAnimation");
}

void Player::moveCard(const EventMoveCard &event) {
    CardZone *startZone = zone(event.startzone());
    if (!startZone)
        return;

    CardZone *targetZone = zone(event.targetzone());
    if (!targetZone)
        return;

    auto &cards = startZone->cards();
    if (size_t(event.id() + 1) > cards.size())
        return;

    QString code = QString::fromStdString(event.code());
    if (code.isEmpty())
        code = cards[event.id()].qcode();

    createMovingCard(code, event.startzone(), event.id(), event.targetzone());
}

void Player::playCard(const EventPlayCard &event) {
    // we trust players to do this themselves for smooth animations
    // so process only opponent's events
    if (!mOpponent)
        return;

    auto &cards = mHand->cards();
    if (size_t(event.handid()) >= cards.size()
        || size_t(event.stageid()) >= mStage->cards().size())
        return;

    QString code = QString::fromStdString(event.code());
    if (code.isEmpty())
        code = cards[event.stageid()].qcode();

    auto cardInfo = CardDatabase::get().getCard(event.code());
    if (!cardInfo)
        return;

    std::string targetZone;
    if (cardInfo->type() == CardType::Climax)
        targetZone = "climax";
    else
        targetZone = "stage";


    createMovingCard(code, "hand", event.handid(), targetZone, event.stageid());
}

void Player::switchStagePositions(const EventSwitchStagePositions &event) {
    if (!mOpponent)
        return;

    if (size_t(event.stageidfrom()) >= mStage->cards().size()
        || size_t(event.stageidto()) >= mStage->cards().size())
        return;

    createMovingCard(mStage->cards()[event.stageidfrom()].qcode(), "stage", event.stageidfrom(),
            "stage", event.stageidto(), true);
}

bool Player::canPlay(Card &card) {
    if (card.level() > mLevel)
        return false;
    if (card.cost() > static_cast<int>(zone("stock")->cards().size()))
        return false;
    if (card.level() > 0 || card.type() == CardType::Climax) {
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

    auto climaxZone = zone("climax");
    if (climaxZone->cards().empty())
        return;
    int handIndex = climaxZone->visualItem()->property("mHandIndex").toInt();
    CommandPlayCard cmd;
    cmd.set_handid(handIndex);
    sendGameCommand(cmd);
}

void Player::setCardAttr(const EventSetCardAttr &event) {
    if (event.stageid() >= 5)
        return;

    mStage->model().setAttr(event.stageid(), event.attr(), event.value());
}

void Player::setCardState(const EventSetCardState &event) {
    if (event.stageid() >= 5)
        return;

    mStage->model().setState(event.stageid(), event.state());
}

void Player::counterStep() {
    if (mOpponent)
        return;

    mGame->counterStep();
}

void Player::levelUp() {
    mLevel++;
    if  (mOpponent)
        return;

    mGame->levelUp();

    auto visualClock = zone("clock")->visualItem();
    QMetaObject::invokeMethod(visualClock, "levelUp");
    visualClock->connect(visualClock, SIGNAL(cardSelected(int)), this, SLOT(cardSelectedForLevelUp(int)));
}

void Player::moveClockToWr() {
    auto clock = zone("clock");
    if (clock->cards().size() < 6)
        return;

    for (int i = 0; i < 6; ++i) {
        bool dontFinishAction = true;
        if (i == 5)
            dontFinishAction = false;
        createMovingCard(clock->cards()[0].qcode(), "clock", 0, "wr", 0, false, dontFinishAction);
    }
}

void Player::endOfAttack() {
    mStage->unhighlightAttacker();
}

void Player::encoreStep() {
    if (mOpponent)
        return;

    mGame->encoreStep();
    mStage->encoreStep();
}

void Player::cardSelectedForLevelUp(int index) {
    mGame->endLevelUp();

    CommandLevelUp cmd;
    cmd.set_clockid(index);
    sendGameCommand(cmd);
}

void Player::sendEncore(int pos) {
    mGame->pauseEncoreStep();
    mStage->deactivateEncoreStep();

    CommandEncoreCharacter cmd;
    cmd.set_stageid(pos);
    sendGameCommand(cmd);
}

void Player::cardPlayed(int handId, int stageId) {
    CommandPlayCard cmd;
    cmd.set_handid(handId);
    cmd.set_stageid(stageId);
    sendGameCommand(cmd);
}

void Player::switchPositions(int from, int to) {
    CommandSwitchStagePositions cmd;
    cmd.set_stageidfrom(from);
    cmd.set_stageidto(to);
    sendGameCommand(cmd);
}

void Player::sendFromStageToWr(int pos) {
    createMovingCard(mStage->cards()[pos].qcode(), "stage", pos, "wr", 0, true, false, true);
}

void Player::testAction()
{
    //createMovingCard("IMC/W43-046", "hand", 1, "res", 0, true);
    if (mStage->cards()[0].cardPresent()) {
        createMovingCard(mStage->cards()[0].qcode(), "stage", 0, "wr");
    }
}

bool Player::playCards(CardModel &hand) {
    auto &cards = mStage->cards();
    for (int i = 0; i < 3; ++i) {
        if (cards[i].cardPresent())
            continue;
        auto &handCards = hand.cards();
        for (int j = 0; (size_t)j < handCards.size(); ++j) {
            if (canPlay(handCards[j]) && handCards[j].type() != CardType::Climax) {
                CommandPlayCard cmd;
                cmd.set_handid(j);
                cmd.set_stageid(i);
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
        if (cards[i].cardPresent() && cards[i].state() == StateStanding) {
            CommandDeclareAttack cmd;
            cmd.set_attacktype(AttackType::FrontAttack);
            cmd.set_stageid(i);
            sendGameCommand(cmd);
            return;
        }
    }

}
