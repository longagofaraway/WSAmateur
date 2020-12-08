#include "player.h"

#include <QVariant>
#include <QMetaObject>
#include <QQuickItem>

#include "gameEvent.pb.h"
#include "moveEvents.pb.h"

Player::Player(size_t id, Game *game, bool opponent)
    : mId(id), mGame(game), mOpponent(opponent) {
    mHand = std::make_unique<Hand>(this, game);
}

void Player::processGameEvent(const std::shared_ptr<GameEvent> event) {
    if (event->event().Is<EventInitialHand>()) {
        EventInitialHand ev;
        event->event().UnpackTo(&ev);
        setInitialHand(ev);
    }
}

void Player::setInitialHand(const EventInitialHand &event) {
    QList<QObject*> handModel;
    for (int i = 0; i < event.codes_size(); ++i)
        handModel.append(new HandCard(QString::fromStdString(event.codes(i)), false));
    mHand->setHand(handModel);
    QMetaObject::invokeMethod(mHand->visualItem(), "startMulligan");
}
