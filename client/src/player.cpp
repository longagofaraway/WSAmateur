#include "player.h"

Player::Player(size_t id, Game *game, bool opponent)
    : mId(id), mGame(game), mOpponent(opponent) {
    mHand = std::make_unique<Hand>(this, game);
}
