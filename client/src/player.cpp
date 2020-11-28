#include "player.h"

Player::Player(Game *game, bool opponent)
    : mGame(game), mOpponent(opponent) {
    mHand = std::make_unique<Hand>(this, game);
}
