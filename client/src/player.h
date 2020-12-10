#pragma once

#include <memory>

#include "hand.h"

class EventInitialHand;
class Game;
class GameEvent;

class Player
{
    size_t mId;
    Game *mGame;
    bool mOpponent;
    std::unique_ptr<Hand> mHand;
public:
    Player(size_t id, Game *game, bool opponent);

    bool isOpponent() {
        return mOpponent;
    }

    size_t id() const { return mId; }
    void processGameEvent(const std::shared_ptr<GameEvent> event);

    void mulliganFinished();

private:
    void setInitialHand(const EventInitialHand &event);
};
