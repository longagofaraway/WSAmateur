#include "player.h"

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

}
