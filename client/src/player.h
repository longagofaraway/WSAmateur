#pragma once

#include <memory>

#include <QObject>

#include <google/protobuf/message.h>

#include "activatedAbilities.h"
#include "cardZone.h"
#include "choiceDialog.h"
#include "deckView.h"

class EventInitialHand;
class EventMoveCard;
class EventPlayCard;
class EventSwitchStagePositions;
class EventDeclareAttack;
class EventSetCardAttr;
class EventSetCardState;
class EventAbilityActivated;
class EventChooseCard;
class EventPlayAbility;
class EventMoveChoice;
class EventDrawChoice;

class Game;
class GameEvent;
class Hand;
class Stage;


class  Player : public QObject
{
    Q_OBJECT
private:
    int mId;
    Game *mGame;
    bool mOpponent;
    bool mActivePlayer = false;
    Hand *mHand;
    Stage *mStage;
    DeckView *mDeckView;
    std::unordered_map<std::string_view, std::unique_ptr<CardZone>> mZones;
    std::unique_ptr<ActivatedAbilities> mAbilityList;
    std::unique_ptr<ChoiceDialog> mChoiceDialog;

    int mLevel = 0;
    int mAttackingId = 0;

public:
    Player(int id, Game *game, bool opponent);
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    bool isOpponent() { return mOpponent; }
    bool activePlayer() const { return mActivePlayer; }
    void setActivePlayer(bool active) { mActivePlayer = active; }
    void setDeck(const std::string &deck);

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
    void sendEndTurnCommand();

    Q_INVOKABLE void cardPlayed(int handId, int stageId);
    Q_INVOKABLE void cardSelectedForLevelUp(int index);
    Q_INVOKABLE void playAbility(int index);
    Q_INVOKABLE void cancelAbility(int index);
    Q_INVOKABLE void chooseCard(int index, QString qzone, bool opponent = false);
    Q_INVOKABLE void sendChoice(int index);

    //test section
    void testAction();
    bool playCards(CardModel &hand);
    void attackWithAll();

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
    void playClimax();
    void setCardAttr(const EventSetCardAttr &event);
    void setCardState(const EventSetCardState &event);
    void counterStep();
    void levelUp();
    void moveClockToWr();
    void endOfAttack();
    void encoreStep();
    void refresh();
    void discardTo7();
    void endGame(bool victory);

    bool canPlay(const Card &card) const;
    bool canPlay(const asn::Ability &a) const;
    bool canPay(const asn::CostItem &c) const;

    void restoreUiState();
    void stopUiInteractions();

    void activateAbilities(const EventAbilityActivated &event);
    void abilityResolved();
    void processChooseCard(const EventChooseCard &event);
    void processSearchCard(const EventSearchCard &event);
    void processMoveChoice(const EventMoveChoice &event);
    void processDrawChoice(const EventDrawChoice &event);
    void processAbilityChoice(const EventAbilityChoice &event);
    void processAbilityGain(const EventAbilityGain &event);
    void processRemoveAbility(const EventRemoveAbility &event);
    void revealTopDeck(const EventRevealTopDeck &event);
    void doneChoosing();
    void makeAbilityActive(const EventPlayAbility &event);
    void conditionNotMet();
    void payCostChoice();

public slots:
    void switchPositions(int from, int to);
    void sendFromStageToWr(int pos);
    void sendAttackDeclaration(int pos, bool sideAttack);
    void sendEncore(int pos);
    void sendDiscardCard(int id);
};
