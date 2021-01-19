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
class EventDeclareAttack;
class EventSetCardAttr;
class EventSetCardState;

class Game;
class GameEvent;
class Hand;
class Stage;

class Player : public QObject
{
    Q_OBJECT
private:
    int mId;
    Game *mGame;
    bool mOpponent;
    bool mActivePlayer = false;
    Hand *mHand;
    Stage *mStage;
    std::unordered_map<std::string_view, std::unique_ptr<CardZone>> mZones;

    int mLevel = 0;

public:
    Player(int id, Game *game, bool opponent);

    bool isOpponent() {
        return mOpponent;
    }
    bool activePlayer() const { return mActivePlayer; }
    void setActivePlayer(bool active) { mActivePlayer = active; }

    int id() const { return mId; }
    CardZone* zone(std::string_view name) const;
    void processGameEvent(const std::shared_ptr<GameEvent> event);
    void sendGameCommand(const google::protobuf::Message &command);

    void mulliganFinished();
    void clockPhaseFinished();
    void mainPhaseFinished();
    void sendClimaxPhaseCommand();
    void sendTakeDamageCommand();
    void sendEncoreCommand();

    //test section
    void testAction();
    void playCards();

private:
    void createMovingCard(const QString &code, const std::string &startZone, int startId,
                          const std::string &targetZone, int targetId = 0, bool isUiAction = false,
                          bool dontFinishAction = false, bool noDelete = false);

    void setInitialHand(const EventInitialHand &event);
    void moveCard(const EventMoveCard &event);
    void playCard(const EventPlayCard &event);
    void switchStagePositions(const EventSwitchStagePositions &event);
    void clockPhase();
    void mainPhase();
    void attackDeclarationStep();
    void declareAttack(const EventDeclareAttack &event);
    void startTurn();
    bool canPlay(Card &card);
    void playClimax();
    void setCardAttr(const EventSetCardAttr &event);
    void setCardState(const EventSetCardState &event);
    void counterStep();
    void levelUp();
    void moveClockToWr();
    void endOfAttack();
    void encoreStep();

public slots:
    void cardPlayed(int handId, int stageId);
    void switchPositions(int from, int to);
    void sendFromStageToWr(int pos);
    void sendAttackDeclaration(int pos, bool sideAttack);
    void cardSelectedForLevelUp(int index);
    void sendEncore(int pos);
};
