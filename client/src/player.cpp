#include "player.h"

#include <QVariant>
#include <QMetaObject>
#include <QQuickItem>

#include "gameEvent.pb.h"
#include "moveEvents.pb.h"

#include "game.h"

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

void Player::mulliganFinished() {
    mHand->endMulligan();


}

void Player::setInitialHand(const EventInitialHand &event) {
    if (!event.codes_size()) {
        for (size_t i = 0; i < event.count(); ++i)
            mHand->addCard({ "cardback", false });
    } else {
        for (int i = 0; i < event.codes_size(); ++i)
            mHand->addCard({ QString::fromStdString(event.codes(i)), false });
    }

    if (mOpponent)
        return;

    QMetaObject::invokeMethod(mGame, "startMulligan");
    mHand->startMulligan();
}
