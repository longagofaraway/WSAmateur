#pragma once

#include <string>
#include <unordered_map>

#include <QMutex>

#include <google/protobuf/message.h>

#include "ability.pb.h"
#include "attackType.pb.h"
#include "gameCommand.pb.h"

#include <abilities.h>

#include "cardImprint.h"
#include "coroutineTask.h"
#include "commands.h"
#include "deckList.h"
#include "globalAbilities/globalAbilities.h"
#include "playerBuffManager.h"
#include "serverCardZone.h"

class ServerGame;
class ServerProtocolHandler;

class CommandMulligan;
class CommandClockPhase;
class CommandPlayCard;
class CommandPlayCounter;
class CommandSwitchStagePositions;
class CommandSwitchPositions;
class CommandDeclareAttack;
class CommandLevelUp;
class CommandEncoreCharacter;
class CommandPlayAct;
class CommandMoveInOrder;

class ProtoTypeCard;

using ZoneMap = std::unordered_map<std::string_view, std::unique_ptr<ServerCardZone>>;

struct TriggeredAbility {
    CardImprint card;
    ProtoAbilityType type;
    int abilityId;
    uint32_t uniqueId = 0;
    std::optional<asn::Ability> ability;
    ServerCard *cardFromTrigger = nullptr;

    asn::Ability getAbility() const;
};

class ServerPlayer
{
    ServerGame *mGame;
    ServerProtocolHandler *mClient;
    int mId;
    bool mReady = false;
    bool mMulliganFinished = false;
    bool mActive = false;
    int mAttacksThisTurn = 0;
    std::unique_ptr<DeckList> mDeck;
    ZoneMap mZones;
    std::vector<ExpectedCommand> mExpectedCommands;

    ServerCard* mAttackingCard = nullptr;
    AttackType mAttackType;
    int mLevel = 0;

    bool mCannotPlayEvents = false;
    bool mCannotPlayBackups = false;
    bool mCharAutoCannotDealDamage = false;
    PlayerBuffManager mBuffManager;

    int mForcedCostReduction = 0;
    int mNextCostReduction = 0;

    // Queue of triggered abilities for check timing
    std::vector<TriggeredAbility> mQueue;
    // Queue of triggered abilities to play outside of check timing
    std::vector<TriggeredAbility> mHelperQueue;

    QMutex mPlayerMutex;

public:
    ServerPlayer(ServerGame *game, ServerProtocolHandler *client, int id);
    ~ServerPlayer();

    void disconnectClient();

    ServerGame* game() { return mGame; }
    int id() const { return mId; }
    bool ready() const { return mReady; }
    void setReady(bool ready);
    bool mulliganFinished() const { return mMulliganFinished; }
    bool active() const { return mActive; }
    void setActive(bool active) { mActive = active; }
    ServerCard* oppositeCard(const ServerCard *card) const;
    ServerCard* attackingCard() { return mAttackingCard; }
    void setAttackingCard(ServerCard *card) { mAttackingCard = card; }
    AttackType attackType() const { return mAttackType; }
    void setAttackType(AttackType type) { mAttackType = type; }
    ServerPlayer* getOpponent();
    int level() const { return mLevel; }
    PlayerBuffManager* buffManager() { return &mBuffManager; }
    ServerCard* cardInBattle();

    void changeAttribute(PlayerAttrType type, bool value);
    bool attribute(PlayerAttrType type) const;
    void sendPlayerAttrChange(PlayerAttrType type, bool value);

    bool cannotPlayEvents() const { return mCannotPlayEvents; }
    bool cannotPlayBackups() const { return mCannotPlayBackups; }
    bool charAutoCannotDealDamage() const { return mCharAutoCannotDealDamage; }
    void setCannotPlayEvents(bool value) { mCannotPlayEvents = value; }
    void setCannotPlayBackups(bool value) { mCannotPlayBackups = value; }
    void setCharAutoCannotDealDamage(bool value) { mCharAutoCannotDealDamage = value; }

    void clearExpectedComands();
    void addExpectedCommand(const std::string &command, int maxCount = 0);
    bool expectsCommand(const GameCommand &command);

    void processGameCommand(GameCommand &cmd);
    void sendGameEvent(const ::google::protobuf::Message &event, int playerId = 0);
    void sendToBoth(const ::google::protobuf::Message &event);

    void addDeck(const std::string &deck);
    DeckList* deck() { return mDeck.get(); }
    ServerCardZone* addZone(std::string_view name, ZoneType type = ZoneType::PublicZone);

    ServerCardZone* zone(std::string_view name);
    ServerCardZone* zone(asn::Zone name);
    const ZoneMap& zones() const { return mZones; }
    void setupZones();
    void createStage();
    void startGame();
    Resumable startTurn();
    void dealStartingHand();
    Resumable mulligan(const CommandMulligan cmd);
    Resumable drawCards(int number);
    void moveCards(std::string_view startZoneName,  const std::vector<int> &cardPositions, std::string_view targetZoneName);
    bool moveCard(std::string_view startZoneName, int startPos, std::string_view targetZoneName, int targetPos = -1,
                  bool reveal = false, bool enableGlobEncore = true);
    bool moveCardToStage(ServerCardZone *startZone, int startPos, ServerCardZone *targetZone, int targetPos);
    bool moveCardToStage(std::unique_ptr<ServerCard> card, const std::string &startZoneName,
                         int startPos, ServerCardZone *stage, int targetPos,
                         std::optional<int> markerPos = {});
    Resumable moveTopDeck(std::string_view targetZoneName);
    void addMarker(ServerCardZone *startZone, int startPos,
                   int targetPos, asn::FaceOrientation faceOrientation);
    void removeMarker(ServerCard *markerBearer, int markerPos, const asn::Place &place,
                      int targetPos = -1);
    Resumable processClockPhaseResult(CommandClockPhase cmd);
    Resumable playCard(const CommandPlayCard cmd);
    Resumable playCounter(const CommandPlayCounter cmd);
    Resumable playCharacter(const CommandPlayCard cmd);
    Resumable playClimax(int handIndex);
    Resumable playEvent(int handIndex);
    Resumable switchPositions(const CommandSwitchStagePositions cmd);
    void switchPositions(int from, int to);
    void swapCards(ServerCard *card1, ServerCard *card2);
    bool canPlay(ServerCard *card);
    bool canPlayCounter(ServerCard *card);
    Resumable climaxPhase();
    bool canAttack();
    Resumable endOfAttack(bool forced = false);
    Resumable startAttackPhase();
    Resumable startOfAttackPhase();
    void attackDeclarationStep();
    Resumable declareAttack(const CommandDeclareAttack cmd);
    Resumable triggerStep(int pos);
    Resumable performTriggerStep(int pos);
    Resumable counterStep();
    Resumable damageStep();
    Resumable levelUp();
    Resumable encoreStep();
    Resumable encoreCharacter(const CommandEncoreCharacter cmd);
    Resumable discardDownTo7();
    void clearClimaxZone();
    Resumable refresh();
    void moveWrToDeck();
    void sendPhaseEvent(asn::Phase phase);
    void sendEndGame(bool victory);
    Resumable processPlayActCmd(const CommandPlayAct cmd);
    void reorderTopCards(const CommandMoveInOrder &cmd, asn::Zone destZone);
    Resumable takeDamage(int damage, ServerCard *card);
    Resumable checkRefreshAndLevelUp();

    void reduceNextCost(int costReduction) { mNextCostReduction += costReduction; }
    void resetCostReduction() { mNextCostReduction = 0; mForcedCostReduction = 0; }
    int costReduction() const { return mNextCostReduction; }
    int forcedCostReduction() const { return mForcedCostReduction; }
    void reduceForcedCostReduction() { mForcedCostReduction--; }

    void removePositionalContBuffsBySource(ServerCard *card);
    void setCardState(ServerCard *card, asn::State state);
    void endOfTurnEffectValidation();

    void checkOnReversed(ServerCard *card);
    void checkOnBattleOpponentReversed(ServerCard *attCard, ServerCard *battleOpponent);
    void checkZoneChangeTrigger(ServerCard *movedCard, std::string_view from, std::string_view to);
    void checkGlobalEncore(ServerCard *movedCard, std::string_view from, std::string_view to);
    void checkOnAttack(ServerCard *attCard);
    void checkOnBeingAttacked(ServerCard *attackTarget, asn::AttackType attackType);
    void checkPhaseTrigger(asn::PhaseState state, asn::Phase phase);
    void checkOnBackup(ServerCard *card);
    void checkOnTriggerReveal(ServerCard *card);
    void checkOnPlayTrigger(ServerCard *card);
    void checkOnDamageCancel(ServerCard *attCard, bool cancelled);
    void checkOnDamageTakenCancel(bool cancelled);
    void checkOtherTrigger(const std::string &code);
    void checkOnActAbility(asn::Player player);
    void triggerBackupAbility(ServerCard *card);
    void triggerRuleAction(RuleAction action, ServerCard *thisCard = nullptr);
    void triggerOnEndOfCardsAttack(ServerCard *card);
    void triggerOnOppCharPlacedByStandby();

    bool canBePlayed(ServerCard *thisCard, const asn::Ability &a);

    Resumable playEventEffects(ServerCard *card);
    Resumable resolveTrigger(ServerCard *card, asn::TriggerIcon trigger);
    Resumable processRuleActions();
    void playContAbilities(ServerCard *card, bool revert = false);
    void resolveAllContAbilities();

    bool hasActivatedAbilities() const;
    bool hasTriggeredRuleActions() const;
    Resumable checkTiming();
    // for abilities that should be played independent of check timing
    // i.e. on paying cost
    Resumable limitedCheckTiming();

    void queueDelayedAbility(const asn::Ability &ability,
                             ServerCard *card,
                             std::string_view cardZone = "",
                             bool helperQueue = false);
    const std::vector<TriggeredAbility>& helperQueue() const { return mHelperQueue; }

private:
    void queueActivatedAbility(const asn::AutoAbility &ability,
                               AbilityState &abilityState,
                               ServerCard *card,
                               std::string_view cardZone = "",
                               ServerCard *cardFromTrigger = nullptr);
    std::vector<ProtoTypeCard> moveMarkersToWr(std::vector<std::unique_ptr<ServerCard>> &markers);

    void sendActivatedAbilitiesToClient(bool helperQueue = false);
    Resumable getAbilityToPlay(std::optional<uint32_t> &uniqueId, bool helperQueue = false);
    Resumable playChosenAbility(uint32_t uniqueId, bool helperQueue = false);
    int stockCostSubstitution(ServerCard *card);
    void sendPlayableCards();

    Resumable testAction();
};
