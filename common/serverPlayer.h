#pragma once

#include <string>
#include <unordered_map>

#include <google/protobuf/message.h>

#include "abilities.pb.h"
#include "attackType.pb.h"
#include "gameCommand.pb.h"

#include "abilities.h"
#include "coroutineTask.h"
#include "commands.h"
#include "deckList.h"
#include "serverCardZone.h"

class ServerGame;
class ServerProtocolHandler;

class CommandMulligan;
class CommandClockPhase;
class CommandPlayCard;
class CommandSwitchStagePositions;
class CommandDeclareAttack;
class CommandLevelUp;
class CommandEncoreCharacter;

struct CardImprint {
    ServerCard *card;
    std::string zone;
    int id = 0;
    bool opponent = false;
    CardImprint() = default;
    CardImprint(const std::string &zone, int id, ServerCard *card = nullptr, bool opponent = false)
        : card(card), zone(zone), id(id), opponent(opponent) {}
};

struct AbilityContext {
    bool mandatory = true;
    bool canceled = false;
    std::vector<CardImprint> chosenCards;
    std::vector<CardImprint> mentionedCards;
    CardImprint thisCard;
    std::optional<asn::Cost> cost;
};

struct TriggeredAbility {
    CardImprint card;
    ProtoAbilityType type;
    int abilityId;
    uint32_t uniqueId;
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

public:
    ServerPlayer(ServerGame *game, ServerProtocolHandler *client, int id);

    int id() const { return mId; }
    bool ready() const { return mReady; }
    void setReady(bool ready) { mReady = ready; }
    bool mulliganFinished() const { return mMulliganFinished; }
    bool active() const { return mActive; }
    void setActive(bool active) { mActive = active; }
    ServerCard* battleOpponent(ServerCard *card) const;
    ServerCard* attackingCard() { return mAttackingCard; }
    void setAttackingCard(ServerCard *card) { mAttackingCard = card; }
    AttackType attackType() const { return mAttackType; }
    void setAttackType(AttackType type) { mAttackType = type; }

    void clearExpectedComands();
    void addExpectedCommand(const std::string &command);
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
    void startTurn();
    void dealStartingHand();
    void mulligan(const CommandMulligan &cmd);
    void drawCards(int number);
    void moveCards(std::string_view startZoneName,  const std::vector<int> &cardIds, std::string_view targetZoneName);
    bool moveCard(std::string_view startZoneName, int id, std::string_view targetZoneName);
    void moveTopDeck(std::string_view targetZoneName);
    Resumable processClockPhaseResult(CommandClockPhase cmd);
    Resumable playCard(const CommandPlayCard &cmd);
    Resumable playCharacter(const CommandPlayCard &cmd);
    void playClimax(int handIndex);
    void switchPositions(const CommandSwitchStagePositions &cmd);
    bool canPlay(ServerCard *card);
    void climaxPhase();
    bool canAttack();
    void endOfAttack();
    void startAttackPhase();
    void attackDeclarationStep();
    Resumable declareAttack(const CommandDeclareAttack &cmd);
    Resumable triggerStep(int pos);
    void counterStep();
    Resumable damageStep();
    Resumable levelUp();
    Resumable encoreStep();
    void encoreCharacter(const CommandEncoreCharacter &cmd);
    Resumable endPhase();
    void refresh();
    void sendEndGame(bool victory);

    void addAttributeBuff(asn::AttributeType attr, int pos, int delta, int duration = 1);
    void setCardState(ServerCard *card, CardState state);

    void endOfTurnEffectValidation();

private:
    // playing abilities
    AbilityContext mContext;
    std::vector<TriggeredAbility> mQueue;

    void checkOnPlacedFromHandToStage(ServerCard *card);

    Resumable checkTiming();

    bool evaluateCondition(const asn::Condition &c);
    bool evaluateConditionIsCard(const asn::ConditionIsCard &c);

    Resumable playAbility(const asn::Ability &a);
    Resumable playAutoAbility(const asn::AutoAbility &a);
    Resumable playEventAbility(const asn::EventAbility &a);
    Resumable playEffect(const asn::Effect &e);
    Resumable playNonMandatory(const asn::NonMandatory &e);
    Resumable playChooseCard(const asn::ChooseCard &e);
    Resumable playMoveCard(const asn::MoveCard &e);
    Resumable playDrawCard(const asn::DrawCard &e);
    void playRevealCard(const asn::RevealCard &e);
    void playAttributeGain(const asn::AttributeGain &e);
};
