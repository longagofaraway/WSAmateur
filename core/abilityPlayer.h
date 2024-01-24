#pragma once

#include <vector>

#include "gameEvent.pb.h"

#include "abilities.h"
#include "cardImprint.h"
#include "coroutineTask.h"

class ServerPlayer;
class CommandSwitchPositions;
class CommandChooseCard;
class Buff;
class ServerCardZone;
struct TriggeredAbility;

struct MoveRecord {
    ServerPlayer *player;
    asn::Zone to;
};

class AbilityPlayer {
    ServerPlayer *mPlayer;

    asn::AbilityType mAbilityType;
    bool mIsCont = false; // ability that is being played is CONT
    bool mIsPayingCost = false; // in the process of paying cost
    bool mRevert = false; // revert effect of cont ability
    bool mRevealChosen = false;
    bool mMandatory = true;
    bool mCanceled = false;
    bool mConditionNotMet = false;
    bool mPerformedInFull = false;
    int mAbilityId;
    std::vector<CardImprint> mChosenCards;
    std::vector<CardImprint> mMentionedCards;
    std::vector<CardImprint> mLastMovedCards;
    CardImprint mThisCard;
    std::optional<asn::Cost> mCost;
    ServerCard *mCardFromTrigger = nullptr;
    std::vector<MoveRecord> mMoveLog;

    std::optional<GameCommand> mLastCommand;

    std::optional<asn::TriggerIcon> mTriggerIcon;

public:
    AbilityPlayer(ServerPlayer *player) : mPlayer(player) {}

    // set this before playing an ability, if applicable
    void setThisCard(const CardImprint &c) { mThisCard = c; }
    void setThisCard(ServerCard *card);

    // must be set if ability is CONT
    void setAbilityId(int abilityId) { mAbilityId = abilityId; }

    // if some card mentioned in the trigger is used in the effect,
    // it needs to be stored
    void setCardFromTrigger(ServerCard *card) { mCardFromTrigger = card; }

    void setTriggerIcon(asn::TriggerIcon icon) { mTriggerIcon = icon; }

    bool canBePlayed(const asn::Ability &a);

    Resumable playAbility(const asn::Ability a);
    void playContAbility(const asn::ContAbility &a, bool &active);
    void revertContAbility(const asn::ContAbility &a);

    int timesCanBePerformed(const asn::Effect &effect);

private:
    ServerPlayer* owner(asn::Player player) const;
    ServerPlayer* owner(ServerCard *card) const;

    bool mandatory() const { return mMandatory; }
    void setMandatory(bool mandatory) { mMandatory = mandatory; }
    bool canceled() const { return mCanceled; }
    void setCanceled(bool canceled) { mCanceled = canceled; }
    bool revert() const { return mRevert; }
    void setRevert(bool revert) { mRevert = revert; }
    bool revealChosen() const { return mRevealChosen; }
    void setRevealChosen(bool revealChosen) { mRevealChosen = revealChosen; }
    int abilityId() const { return mAbilityId; }
    const asn::Cost& cost() const { return *mCost; }
    bool hasCost() const { return mCost.has_value(); }
    void setCost(const asn::Cost &cost) { mCost = cost; }
    CardImprint& thisCard() { return mThisCard; }
    bool cont() const { return mIsCont; }
    void setCont(bool cont) { mIsCont = cont; }
    bool isPayingCost() const { return mIsPayingCost; }
    void setPayingCost(bool p) { mIsPayingCost = p; }

    void addChosenCard(CardImprint &&c) { mChosenCards.emplace_back(std::move(c)); }
    std::vector<CardImprint>& chosenCards() { return mChosenCards; }
    void clearChosenCards() { mChosenCards.clear(); }

    void addMentionedCard(CardImprint &&c) { mMentionedCards.emplace_back(std::move(c)); }
    std::vector<CardImprint>& mentionedCards() { return mMentionedCards; }
    void clearMentionedCards() { mMentionedCards.clear(); }
    void removeMentionedCard(int cardId);

    void addLastMovedCard(CardImprint &&c) { mLastMovedCards.emplace_back(std::move(c)); }
    std::vector<CardImprint>& lastMovedCards() { return mLastMovedCards; }
    void clearLastMovedCards() { mLastMovedCards.clear(); }

    ServerCard* cardFromTrigger() { return mCardFromTrigger; }

    bool canBePayed(const asn::CostItem &c);

    Resumable playAutoAbility(const asn::AutoAbility &a);
    Resumable playActAbility(const asn::ActAbility &a);
    Resumable playEventAbility(const asn::EventAbility &a);

    void playContEffect(const asn::Effect &e);
    Resumable playEffect(const asn::Effect &e, std::optional<asn::Effect> nextEffect = {});
    Resumable playEffects(const std::vector<asn::Effect> &e);
    Resumable playTriggeredAbilities(const std::vector<TriggeredAbility> &abilities);
    Resumable playNonMandatory(const asn::NonMandatory &e);
    Resumable playChooseCard(const asn::ChooseCard &e, bool clearPrevious = true);
    Resumable playMoveCard(const asn::MoveCard &e);
    Resumable playDrawCard(const asn::DrawCard &e);
    Resumable playRevealCard(const asn::RevealCard &e, std::optional<asn::Effect> nextEffect = {});
    void playAttributeGain(const asn::AttributeGain &e);
    Resumable playPayCost(const asn::PayCost &e);
    Resumable payCost();
    Resumable playSearchCard(const asn::SearchCard &e);
    void playShuffle(const asn::Shuffle &e);
    Resumable playAbilityGain(const asn::AbilityGain &e);
    void playTemporaryAbilityGain(const asn::AbilityGain &e);
    Resumable playPerformEffect(const asn::PerformEffect &e);
    void playMoveWrToDeck(const asn::MoveWrToDeck &e);
    Resumable playChangeState(const asn::ChangeState &e);
    Resumable playFlipOver(const asn::FlipOver &e);
    void playBackup(const asn::Backup &e);
    void playTriggerCheckTwice();
    Resumable playLook(const asn::Look &e, std::optional<asn::Effect> nextEffect = {});
    Resumable playLookRevealCommon(asn::EffectType type, int numCards,
                                   const std::optional<asn::Effect> &nextEffect,
                                   asn::Player zoneOwner = asn::Player::Player);
    void playEarlyPlay();
    void playCannotPlay();
    void playCannotUseBackupOrEvent(const asn::CannotUseBackupOrEvent &e);
    Resumable playDealDamage(const asn::DealDamage &e, bool isShotTrigger = false);
    Resumable playSwapCards(const asn::SwapCards &e);
    void playCannotAttack(const asn::CannotAttack &e);
    void playCannotBecomeReversed(const asn::CannotBecomeReversed &e);
    void playCannotBeChosen(const asn::CannotBeChosen &e);
    void playOpponentAutoCannotDealDamage(const asn::OpponentAutoCannotDealDamage &e);
    Resumable playStockSwap(const asn::StockSwap &e);
    void playCannotMove(const asn::CannotMove &e);
    void playCannotStand(const asn::CannotStand &e);
    void playSideAttackWithoutPenalty(const asn::SideAttackWithoutPenalty &e);
    Resumable playPutOnStageRested(const asn::PutOnStageRested &e);
    Resumable playAddMarker(const asn::AddMarker &e);
    Resumable playRemoveMarker(const asn::RemoveMarker &e);
    void playTriggerGain(const asn::TriggerIconGain &e);
    void playCanPlayWithoutColorRequirement(const asn::CanPlayWithoutColorRequirement &e);
    void playDelayedAbility(const asn::DelayedAbility &e);
    Resumable playCostSubstitution(const asn::CostSubstitution &e);
    void playSkipPhase(const asn::SkipPhase &e);

    Resumable playOtherEffect(const asn::OtherEffect &e);
    Resumable playS79_20();

    bool evaluateCondition(const asn::Condition &c);
    bool evaluateConditionIsCard(const asn::ConditionIsCard &c);
    bool evaluateConditionHaveCard(const asn::ConditionHaveCard &c);
    bool evaluateConditionAnd(const asn::ConditionAnd &c);
    bool evaluateConditionInBattleWithThis();
    bool evaluateConditionDuringCardsFirstTurn();
    bool evaluateConditionSumOfLevels(const asn::ConditionSumOfLevels &c);
    bool evaluateConditionDuringTurn(const asn::ConditionDuringTurn &c);
    bool evaluateConditionCheckMilledCards(const asn::ConditionCheckMilledCards &c);
    bool evaluateConditionCardsLocation(const asn::ConditionCardsLocation &c);
    bool evaluateConditionRevealedCard(const asn::ConditionRevealCard &c);
    bool evaluateConditionPlayersLevel(const asn::ConditionPlayersLevel &c);
    bool evaluateConditionCardMoved(const asn::ConditionCardMoved &c);
    bool evaluateConditionPerformedInFull();
    bool evaluateConditionHasMarkers(const asn::ConditionHasMarkers &c);

    void applyBuff(std::vector<ServerCard*> &targets, Buff &buff);
    void sendLookCard(ServerCard *card);
    void sendRevealCard(ServerCard *card);
    std::map<int, ServerCard*> processCommandChooseCard(const CommandChooseCard &cmd);
    Resumable getStagePosition(int &position, const asn::MoveCard &e);
    Resumable moveFromTop(const asn::MoveCard &e, int toZoneIndex, int toIndex);
    void setCannotPlayBackupOrEvent(ServerPlayer *player, asn::BackupOrEvent type);
    int getForEachMultiplierValue(const asn::Multiplier &m);
    int getTriggerNumberMultiplierValue(const asn::Multiplier &m);
    std::vector<ServerCard*> getTargets(const asn::Target &t,
                                        asn::PlaceType placeType = asn::PlaceType::SpecificPlace,
                                        std::optional<asn::Place> place = {});
    std::vector<ServerCard *> getTargetsFromAllZones(const asn::Target &t);
    bool checkTarget(const asn::TargetSpecificCards &spec, ServerCard *card);
    bool findChooseTargetsAutomatically(const asn::ChooseCard &e);
    Resumable waitForPlayerLookConfirmation();

    bool checkHinaCondition(const asn::ChooseCard &e, const CommandChooseCard& cmd, ServerCardZone* zone);
    bool areChosenCardsValid(const asn::ChooseCard &e, const CommandChooseCard& cmd, ServerCardZone* zone);

    void logMove(ServerPlayer *player, asn::Zone toZone);

public:
    int getAddLevelMultiplierValue(const asn::Multiplier &m);
};
