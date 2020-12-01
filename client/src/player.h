#pragma once

#include <memory>

#include "hand.h"

class Game;

class Player {
    size_t mId;
    Game *mGame;
    bool mOpponent;
    std::unique_ptr<Hand> mHand;
public:
    Player(size_t id, Game *game, bool opponent);

    bool isOpponent() {
        return mOpponent;
    }

protected:
};
