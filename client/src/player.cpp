#include "player.h"

#include <QVariant>
#include <QMetaObject>
#include <QQuickItem>

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

Player::Player(size_t id, Game *game, bool opponent)
    : mId(id), mGame(game), mOpponent(opponent) {
    auto hand = std::make_unique<Hand>(this, game);
    mHand = hand.get();
    mZones.emplace("hand", std::move(hand));

    auto wr = std::make_unique<WaitingRoom>(this, game);
    mZones.emplace("wr", std::move(wr));
    auto deck = std::make_unique<Deck>(this, game);
    mZones.emplace("deck", std::move(deck));
    auto clock = std::make_unique<CommonCardZone>(this, game, "Clock");
    clock->model().addCard(std::string("IMC/W43-046"));
    clock->model().addCard(std::string("IMC/W43-046"));
    mZones.emplace("clock", std::move(clock));
    auto stock = std::make_unique<CommonCardZone>(this, game, "Stock");
    stock->model().addCard();
    stock->model().addCard();
    mZones.emplace("stock", std::move(stock));
    auto stage= std::make_unique<Stage>(this, game);
    mStage = stage.get();
    mZones.emplace("stage", std::move(stage));
    auto level = std::make_unique<CommonCardZone>(this, game, "Level");
    level->model().addCard(std::string("IMC/W43-046"));
    mZones.emplace("level", std::move(level));
    auto climax = std::make_unique<CommonCardZone>(this, game, "Climax");
    //climax->model().addCard(std::string("IMC/W43-127"));
    mZones.emplace("climax", std::move(climax));
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
    } else if (event->event().Is<EventAttackPhase>()) {
        attackPhase();
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

void Player::attackPhase() {
    if (mOpponent)
        return;

    mGame->attackPhase();
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
    // 3. Player sends 'play climax' command. Climax effects are rsolved.
    // If there are 'At the start of climax phase' abilities that mess with card count in hand or
    // smth like that we are fcked;
    mHand->endMainPhase();
    mStage->endMainPhase();
    CommandClimaxPhase cmd;
    sendGameCommand(cmd);
}

void Player::setInitialHand(const EventInitialHand &event) {
    if (!event.codes_size()) {
        for (size_t i = 0; i < event.count(); ++i)
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
                              const std::string &targetZone, int targetId, bool isUiAction) {
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
    if (event.id() + 1 > cards.size())
        return;

    QString code = QString::fromStdString(event.code());
    if (code.isEmpty())
        code = cards[event.id()].qcode();

    createMovingCard(code, event.startzone(), event.id(), event.targetzone());
    startZone->removeCard(event.id());
}

void Player::playCard(const EventPlayCard &event) {
    // we trust players to do this themselves for smooth animations
    // so process only opponent's events
    if (!mOpponent)
        return;

    auto &cards = mHand->cards();
    if (event.handid() >= cards.size()
        || event.stageid() >= mStage->cards().size())
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
    mHand->removeCard(event.handid());
}

void Player::switchStagePositions(const EventSwitchStagePositions &event) {
    if (!mOpponent)
        return;

    if (event.stageidfrom() >= mStage->cards().size()
        || event.stageidto() >= mStage->cards().size())
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
        // TODO same for level zone
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
    createMovingCard(mStage->cards()[pos].qcode(), "stage", pos, "wr", 0, true);
}

void Player::testAction()
{
    auto &cards = mHand->cards();
    for (size_t i = 0; i < cards.size(); ++i) {
        CommandPlayCard cmd;
        cmd.set_handid(0);
        sendGameCommand(cmd);
    }
}
