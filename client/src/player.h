#pragma once

#include <memory>

#include <QObject>

#include <google/protobuf/message.h>

#include "activatedAbilities.h"
#include "cardZone.h"
#include "choiceDialog.h"
#include "deckList.h"
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
class EventStartResolvingAbility;
class EventSearchCard;
class EventMoveDestinationChoice;
class EventMoveDestinationIndexChoice;
class EventMoveTargetChoice;
class EventAbilityChoice;
class EventEffectChoice;
class EventAbilityGain;
class EventRemoveAbility;
class EventLook;
class EventReveal;
class EventRevealTopDeck;
class EventLookTopDeck;
class EventSetCannotPlay;
class EventSetCardStateChoice;
class EventSetCardStateTargetChoice;
class EventSetCardBoolAttr;
class EventSetPlayerAttr;
class EventRevealFromHand;
class EventAddMarker;
class EventPlayableCards;
class ProtoTypeCard;

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
    DeckView *mDeckView;
    bool mDeckSet = false;
    DeckList mDeckList;
    std::unordered_map<std::string_view, std::unique_ptr<CardZone>> mZones;
    std::unique_ptr<ActivatedAbilities> mAbilityList;
    std::unique_ptr<ChoiceDialogBase> mChoiceDialog;
    std::unordered_map<int, std::unique_ptr<CardZone>> mMarkerViews;

    int mLevel = 0;
    int mAttackingPos = 0;

    bool mCannotPlayEvents = false;
    bool mCannotPlayBackups = false;

    bool mResolvingAbilities = false;

public:
    Player(int id, Game *game, bool opponent);
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    bool isOpponent() { return mOpponent; }
    bool activePlayer() const { return mActivePlayer; }
    void setActivePlayer(bool active) { mActivePlayer = active; }
    void setDeck(const std::string &deck);
    void setDeck(const DeckList &deck);
    Player* getOpponent() const;
    bool deckSet() const { return mDeckSet; }

    int id() const { return mId; }
    int level() const { return mLevel; }
    CardZone* zone(std::string_view name) const;
    CardZone* zone(asn::Zone zone_) const;

    void processGameEvent(const std::shared_ptr<GameEvent> event);
    void sendGameCommand(const google::protobuf::Message &command);

    void mulliganFinished();
    void clockPhaseFinished();
    void mainPhaseFinished();
    void counterStepFinished();
    void sendTakeDamageCommand();
    void sendEncoreCommand();
    void sendEndTurnCommand();

    Q_INVOKABLE void cardPlayed(int handId, int stageId);
    Q_INVOKABLE void cardSelectedForLevelUp(int index);
    Q_INVOKABLE void playAbility(int index);
    Q_INVOKABLE void cancelAbility(int index);
    Q_INVOKABLE void chooseCard(int index, QString qzone, bool opponent = false);
    Q_INVOKABLE void chooseCardOrPosition(int index, QString qzone, bool opponent = false);
    Q_INVOKABLE void sendChoice(int index);
    Q_INVOKABLE void playActAbility(int index);
    Q_INVOKABLE void sendPlayCounter(int handId);
    Q_INVOKABLE void addCard(int id, QString code, QString zoneName, int targetPos = -1);
    Q_INVOKABLE void interactWithDeck();
    Q_INVOKABLE void cardInserted(QString startZone, QString targetZone);
    Q_INVOKABLE void createMarkerView(int index);

    void resetChoiceDialog();

    bool hasActivatedAbilities() const { return mAbilityList->count(); }
    ActivatedAbility& activeAbility() { return mAbilityList->ability(mAbilityList->activeId()); }

    std::vector<std::string> cardReferences(const std::string &code) const;

    //test section
    void testAction();
    bool playCards(CardModel &hand);
    void attackWithAll();

private:
    void createMovingCard(int id, const QString &code, const std::string &startZone, int startPos,
                          const std::string &targetZone, int targetPos = 0, bool isUiAction = false,
                          bool dontFinishAction = false, bool noDelete = false);
    void moveCard(int id, const std::string &code,
                  const std::string &startZoneName, int startPos,
                  const std::string &targetZoneName, int targetPos,
                  const std::vector<ProtoTypeCard> markers);

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
    bool canPlay(const Card &thisCard, const asn::Ability &a) const;
    bool canPay(const Card &thisCard, const asn::CostItem &c) const;
    bool canPlayCounter(const Card &card) const;

    void restoreUiState();
    void stopUiInteractions();

    using OptionalPlace = std::optional<std::reference_wrapper<const asn::Place>>;
    int highlightCardsFromEvent(const EventChooseCard &event, const asn::ChooseCard &effect);
    int highlightCardsForChoice(const asn::Target &target, const asn::Place &place,
                                const std::optional<asn::ChooseCard> &chooseEffect = {});
    void dehighlightCards(asn::PlaceType placeType, OptionalPlace place);
    void highlightPlayableCards();

    void activateAbilities(const EventAbilityActivated &event);
    void startResolvingAbility(const EventStartResolvingAbility &event);
    void endResolvingAbilties();
    void abilityResolved();
    void processChooseCard(const EventChooseCard &event);
    void processChooseCardInternal(int eligibleCount, OptionalPlace place, bool mandatory, asn::Player executor);
    void sendChooseCard(const asn::ChooseCard &e);
    void sendChooseCard(const asn::SearchCard &e);
    void processSearchCard(const EventSearchCard &event);
    void processMoveChoice(const EventMoveChoice &event);
    void processMoveDestinationChoice(const EventMoveDestinationChoice &event);
    void processMoveDestinationIndexChoice(const EventMoveDestinationIndexChoice &event);
    void processMoveTargetChoice(const EventMoveTargetChoice &event);
    void processDrawChoice(const EventDrawChoice &event);
    void processAbilityChoice(const EventAbilityChoice &event);
    void processEffectChoice(const EventEffectChoice &event);
    void processAbilityGain(const EventAbilityGain &event);
    void processRemoveAbility(const EventRemoveAbility &event);
    void processLook(const EventLook &event);
    void processReveal(const EventReveal &event);
    void processLookRevealCommon(asn::EffectType nextEffectType, const std::string &nextEffectBuf);
    void processLookRevealNextCard(asn::EffectType type);
    void revealTopDeck(const EventRevealTopDeck &event);
    void lookTopDeck(const EventLookTopDeck &event);
    void doneChoosing();
    void makeAbilityActive(const EventPlayAbility &event);
    void conditionNotMet();
    void payCostChoice();
    void setCannotPlay(const EventSetCannotPlay &event);
    void setPlayerAttr(const EventSetPlayerAttr &event);
    void processSetCardStateChoice(const EventSetCardStateChoice &event);
    void processSetCardStateTargetChoice(const EventSetCardStateTargetChoice &event);
    void processSetCardBoolAttr(const EventSetCardBoolAttr &event);
    void processRevealFromHand(const EventRevealFromHand &event);
    void processRuleActionChoice();
    void processAddMarker(const EventAddMarker &event);
    void processPlayableCards(const EventPlayableCards &event);

    const Card& correspondingCard(const ActivatedAbility &abilityDescriptor);
    std::vector<const Card*> getTargets(const Card &thisCard, const asn::Target &t,
                                        asn::Zone from_zone = asn::Zone::Stage) const;
    void fillReferenceCache();
    void setDeckInternal();

    void toggleZoneView(const asn::Place &place, bool open);

public slots:
    void sendSwitchPositions(int from, int to);
    void sendFromStageToWr(int pos);
    void sendAttackDeclaration(int pos, bool sideAttack);
    void sendEncore(int pos);
    void sendDiscardCard(int id);
    void sendPlayActAbility(int cardPos, int abilityId);
    void deleteMarkerView(int index);

signals:
    void unrecoverableError(QString message);

private:
    bool mPlayingClimax = false;
    int mClimaxId;
    std::unordered_multimap<std::string, std::string> cardReferenceCache; // code to code
};
