#pragma once

#include <memory>

#include "hand.h"

class Game;

class Player {
    Game *mGame;
    bool mOpponent;
    std::unique_ptr<Hand> mHand;
public:
    Player(Game *game, bool opponent);

    bool isOpponent() {
        return mOpponent;
    }

protected:
};
