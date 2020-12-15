#pragma once

#include <memory>

#include <google/protobuf/message.h>

#include "cardZone.h"
#include "hand.h"

class EventInitialHand;
class EventDrawCard;
class EventMoveCard;
class Game;
class GameEvent;

class Player
{
    size_t mId;
    Game *mGame;
    bool mOpponent;
    bool mActivePlayer = false;
    Hand *mHand;
    std::unordered_map<std::string_view, std::unique_ptr<CardZone>> mZones;

public:
    Player(size_t id, Game *game, bool opponent);

    bool isOpponent() {
        return mOpponent;
    }
    bool activePlayer() const { return mActivePlayer; }
    void setActivePlayer(bool active) { mActivePlayer = active; }

    size_t id() const { return mId; }
    CardZone* zone(std::string_view name) const;
    void processGameEvent(const std::shared_ptr<GameEvent> event);
    void sendGameCommand(const google::protobuf::Message &command);

    void mulliganFinished();
    void clockPhaseFinished();

private:
    void createMovingCard(const EventMoveCard &event, const QString &code);


    void setInitialHand(const EventInitialHand &event);
    void moveCard(const EventMoveCard &event);
    void clockPhase();
    void startTurn();
};
