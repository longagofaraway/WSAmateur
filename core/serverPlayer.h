#pragma once

#include <string>
#include <unordered_map>

#include <google/protobuf/message.h>

#include "ability.pb.h"
#include "attackType.pb.h"
#include "gameCommand.pb.h"

#include <abilities.h>

#include "cardImprint.h"
#include "coroutineTask.h"
#include "commands.h"
#include "deckList.h"
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

struct TriggeredAbility {
    CardImprint card;
    ProtoAbilityType type;
    int abilityId;
    uint32_t uniqueId = 0;
    std::optional<asn::Ability> ability;

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
    std::unique_ptr<DeckList> mDeck;
    std::unordered_map<std::string_view, std::unique_ptr<ServerCardZone>> mZones;
    std::vector<ExpectedCommand> mExpectedCommands;

    ServerCard* mAttackingCard = nullptr;
    AttackType mAttackType;
    int mLevel = 0;

    bool mCanPlayEvents = true;
    bool mCanPlayBackups = true;

    std::vector<TriggeredAbility> mQueue;

public:
    ServerPlayer(ServerGame *game, ServerProtocolHandler *client, int id);

    ServerGame* game() { return mGame; }
    int id() const { return mId; }
    bool ready() const { return mReady; }
    void setReady(bool ready) { mReady = ready; }
    bool mulliganFinished() const { return mMulliganFinished; }
    bool active() const { return mActive; }
    void setActive(bool active) { mActive = active; }
    ServerCard* oppositeCard(ServerCard *card) const;
    ServerCard* attackingCard() { return mAttackingCard; }
    void setAttackingCard(ServerCard *card) { mAttackingCard = card; }
    AttackType attackType() const { return mAttackType; }
    void setAttackType(AttackType type) { mAttackType = type; }
    ServerPlayer* getOpponent();
    int level() const { return mLevel; }

    bool canPlayEvents() const { return mCanPlayEvents; }
    bool canPlayBackups() const { return mCanPlayBackups; }
    void setCanPlayEvents(bool value) { mCanPlayEvents = value; }
    void setCanPlayBackups(bool value) { mCanPlayBackups = value; }

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
    void setupZones();
    void createStage();
    void startGame();
    Resumable startTurn();
    void dealStartingHand();
    Resumable mulligan(const CommandMulligan &cmd);
    void drawCards(int number);
    void moveCards(std::string_view startZoneName,  const std::vector<int> &cardPositions, std::string_view targetZoneName);
    bool moveCard(std::string_view startZoneName, int startPos, std::string_view targetZoneName, int targetPos = -1,
                  bool reveal = false, bool enableGlobEncore = true);
    bool moveCardToStage(ServerCardZone *startZone, int startPos, ServerCardZone *targetZone, int targetPos);
    void moveTopDeck(std::string_view targetZoneName);
    Resumable processClockPhaseResult(CommandClockPhase cmd);
    Resumable playCard(const CommandPlayCard &cmd);
    Resumable playCounter(const CommandPlayCounter &cmd);
    Resumable playCharacter(const CommandPlayCard &cmd);
    Resumable playClimax(int handIndex);
    Resumable playEvent(int handIndex);
    void switchPositions(const CommandSwitchStagePositions &cmd);
    void switchPositions(int from, int to);
    bool canPlay(ServerCard *card);
    void climaxPhase();
    bool canAttack();
    void endOfAttack();
    Resumable startAttackPhase();
    Resumable startOfAttackPhase();
    void attackDeclarationStep();
    Resumable declareAttack(const CommandDeclareAttack &cmd);
    Resumable triggerStep(int pos);
    Resumable performTriggerStep(int pos);
    void counterStep();
    Resumable damageStep();
    Resumable levelUp();
    Resumable encoreStep();
    Resumable encoreCharacter(const CommandEncoreCharacter &cmd);
    Resumable endPhase();
    void refresh();
    void moveWrToDeck();
    void sendPhaseEvent(asn::Phase phase);
    void sendEndGame(bool victory);
    Resumable processPlayActCmd(const CommandPlayAct &cmd);
    void reorderTopCards(const CommandMoveInOrder &cmd, asn::Zone destZone);
    int addAbilityToCard(ServerCard *card, const asn::Ability &a, int duration);
    void removeAbilityFromCard(ServerCard *card, int abilityId);
    Resumable takeDamage(int damage);

    void sendAttrChange(ServerCard *card, asn::AttributeType attr);
    void sendChangedAttrs(ServerCard *card, std::tuple<int, int, int> oldAttrs);
    void addAttributeBuff(ServerCard *card, asn::AttributeType attr, int delta, int duration = 1);
    void addContAttributeBuff(ServerCard *card,
                              ServerCard *source,
                              int abilityId,
                              asn::AttributeType attr,
                              int delta,
                              bool positional = false);
    void removeContAttributeBuff(ServerCard *card, ServerCard *source, int abilityId, asn::AttributeType attr);
    void removePositionalContBuffsBySource(ServerCard *card);
    void removePositionalContBuffsFromCard(ServerCard *card);
    void addAbilityAsContBuff(ServerCard *card,
                              ServerCard *source,
                              int sourceAbilityId,
                              const asn::Ability &ability,
                              bool positional = false);
    void removeAbilityAsContBuff(ServerCard *card, ServerCard *source, int sourceAbilityId);
    void setCardState(ServerCard *card, CardState state);
    void endOfTurnEffectValidation();

    void checkOnReversed(ServerCard *card);
    void checkOnBattleOpponentReversed(ServerCard *attCard, ServerCard *battleOpponent);
    void checkZoneChangeTrigger(ServerCard *movedCard, std::string_view from, std::string_view to);
    void checkGlobalEncore(ServerCard *movedCard, std::string_view from, std::string_view to);
    void checkOnAttack(ServerCard *attCard);
    void checkPhaseTrigger(asn::PhaseState state, asn::Phase phase);
    void checkOnBackup(ServerCard *card);
    void checkOnTriggerReveal(ServerCard *card);
    void checkOnPlayTrigger(ServerCard *card);
    void checkOtherTrigger(const std::string &code);
    void triggerBackupAbility(ServerCard *card);

    bool canBePlayed(ServerCard *thisCard, const asn::Ability &a);

    Resumable playEventEffects(ServerCard *card);
    Resumable resolveTrigger(ServerCard *card, asn::TriggerIcon trigger);
    Resumable processRuleActions(bool &ruleActionFound);
    void playContAbilities(ServerCard *card, bool revert = false);
    void resolveAllContAbilities();
    void deactivateContAbilities(ServerCard *source);

    bool hasActivatedAbilities() const;
    Resumable checkTiming();

private:
    void queueActivatedAbility(const asn::AutoAbility &ability,
                               AbilityState &abilityState,
                               ServerCard *card,
                               std::string_view cardZone = "");
};
