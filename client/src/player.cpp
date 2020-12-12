#include "player.h"

#include <QVariant>
#include <QMetaObject>
#include <QQuickItem>

#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"

#include "deck.h"
#include "game.h"
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
    }
}

void Player::sendGameCommand(const google::protobuf::Message &command) {
    mGame->sendGameCommand(command, mId);
}

void Player::mulliganFinished() {
    mHand->endMulligan();
    auto cards = mHand->cards();
    CommandMulligan cmd;
    for (uint32_t i = 0; i < cards.size(); ++i) {
        if (cards[i].glow())
            cmd.add_ids(i);
    }

    sendGameCommand(cmd);
}

void Player::setInitialHand(const EventInitialHand &event) {
    if (!event.codes_size()) {
        for (size_t i = 0; i < event.count(); ++i)
            mHand->addCard({ "cardback" });
    } else {
        for (int i = 0; i < event.codes_size(); ++i)
            mHand->addCard({ event.codes(i) });
    }

    if (mOpponent)
        return;

    QMetaObject::invokeMethod(mGame, "startMulligan");
    mHand->startMulligan();
}

void Player::createMovingCard(const EventMoveCard &event, const QString &code) {
    QString source = code;
    if (source.isEmpty())
        source = "cardback";
    QQmlComponent component(mGame->engine(), "qrc:/qml/MovingCard.qml");
    QQuickItem *obj = qobject_cast<QQuickItem*>(component.create(mGame->context()));
    obj->setParentItem(mGame->parentItem());
    obj->setProperty("code", code);
    obj->setProperty("opponent", mOpponent);
    obj->setProperty("source", "image://imgprov/" + source);
    obj->setProperty("startZone", QString::fromStdString(event.startzone()));
    obj->setProperty("startId", event.id());
    obj->setProperty("targetZone", QString::fromStdString(event.targetzone()));
    obj->connect(obj, SIGNAL(moveFinished()), mGame, SLOT(cardMoveFinished()));
    mGame->startUiAction();
    QMetaObject::invokeMethod(obj, "startAnimation");
}
void Player::moveCard(const EventMoveCard &event) {
    CardZone *startZone = zone(event.startzone());
    if (!startZone)
        return;

    CardZone *targetZone = zone(event.targetzone());
    if (!targetZone)
        return;

    auto cards = startZone->cards();
    if (event.id() + 1 > cards.size())
        return;

    QString code = QString::fromStdString(event.code());
    if (code.isEmpty())
        code = cards[event.id()].qcode();

    createMovingCard(event, code);
    startZone->removeCard(event.id());
}
