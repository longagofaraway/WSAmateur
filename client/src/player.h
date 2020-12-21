#pragma once

#include <memory>

#include <QObject>

#include <google/protobuf/message.h>

#include "cardZone.h"

class EventInitialHand;
class EventDrawCard;
class EventMoveCard;
class EventPlayCard;
class EventSwitchStagePositions;
class Game;
class GameEvent;
class Hand;
class Stage;

class Player : public QObject
{
    Q_OBJECT
private:
    size_t mId;
    Game *mGame;
    bool mOpponent;
    bool mActivePlayer = false;
    Hand *mHand;
    Stage *mStage;
    std::unordered_map<std::string_view, std::unique_ptr<CardZone>> mZones;

    int mLevel = 0;

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

    void testAction();

private:
    void createMovingCard(const QString &code, const std::string &startZone, int startId,
                          const std::string &targetZone, int targetId = 0, bool isUiAction = false);

    void setInitialHand(const EventInitialHand &event);
    void moveCard(const EventMoveCard &event);
    void playCard(const EventPlayCard &event);
    void switchStagePositions(const EventSwitchStagePositions &event);
    void clockPhase();
    void mainPhase();
    void startTurn();
    bool canPlay(Card &card);

public slots:
    void cardPlayed(int handId, int stageId);
    void switchPositions(int from, int to);
    void sendFromStageToWr(int pos);
};
