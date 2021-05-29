#pragma once

#include <vector>

#include "gameEvent.pb.h"

#include "abilities.h"
#include "cardImprint.h"
#include "coroutineTask.h"

class ServerPlayer;
class CommandSwitchPositions;

class AbilityPlayer {
    ServerPlayer *mPlayer;
    bool mMandatory = true;
    bool mCanceled = false;
    bool mRevealChosen = false;
    bool mRevert = false; // revert effect of cont ability
    bool mConditionNotMet = false;
    int mAbilityId;
    std::vector<CardImprint> mChosenCards;
    std::vector<CardImprint> mMentionedCards;
    std::vector<CardImprint> mLastMovedCards;
    CardImprint mThisCard;
    std::optional<asn::Cost> mCost;

    std::optional<GameCommand> mLastCommand;

public:
    AbilityPlayer(ServerPlayer *player) : mPlayer(player) {}

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
    bool abilityId() const { return mAbilityId; }
    void setAbilityId(int abilityId) { mAbilityId = abilityId; }
    const asn::Cost& cost() const { return *mCost; }
    bool hasCost() const { return mCost.has_value(); }
    void setCost(const asn::Cost &cost) { mCost = cost; }
    CardImprint& thisCard() { return mThisCard; }
    void setThisCard(const CardImprint &c) { mThisCard = c; }
    void addChosenCard(CardImprint &&c) { mChosenCards.emplace_back(std::move(c)); }
    std::vector<CardImprint>& chosenCards() { return mChosenCards; }
    void clearChosenCards() { mChosenCards.clear(); }

    void addMentionedCard(CardImprint &&c) { mMentionedCards.emplace_back(std::move(c)); }
    std::vector<CardImprint>& mentionedCards() { return mMentionedCards; }
    void clearMentionedCards() { mMentionedCards.clear(); }
    void removeMentionedCard(int cardPos);

    void addLastMovedCard(CardImprint &&c) { mLastMovedCards.emplace_back(std::move(c)); }
    std::vector<CardImprint>& lastMovedCards() { return mLastMovedCards; }
    void clearLastMovedCards() { mLastMovedCards.clear(); }

    Resumable playAbility(const asn::Ability &a);
    Resumable playAutoAbility(const asn::AutoAbility &a);
    Resumable playActAbility(const asn::ActAbility &a);
    Resumable playEventAbility(const asn::EventAbility &a);

    void playContAbility(const asn::ContAbility &a, bool &active);
    void playContEffect(const asn::Effect &e);
    Resumable playEffect(const asn::Effect &e, std::optional<asn::Effect> nextEffect = {});
    Resumable playEffects(const std::vector<asn::Effect> &e);
    Resumable playNonMandatory(const asn::NonMandatory &e);
    Resumable playChooseCard(const asn::ChooseCard &e);
    Resumable playMoveCard(const asn::MoveCard &e);
    Resumable playDrawCard(const asn::DrawCard &e);
    void playRevealCard(const asn::RevealCard &e);
    void playAttributeGain(const asn::AttributeGain &e, bool cont = false);
    Resumable playPayCost(const asn::PayCost &e);
    Resumable payCost();
    Resumable playSearchCard(const asn::SearchCard &e);
    void playShuffle(const asn::Shuffle &e);
    Resumable playAbilityGain(const asn::AbilityGain &e);
    Resumable playPerformEffect(const asn::PerformEffect &e);
    void playMoveWrToDeck(const asn::MoveWrToDeck &e);
    void playChangeState(const asn::ChangeState &e);
    Resumable playFlipOver(const asn::FlipOver &e);
    void playBackup(const asn::Backup &e);
    void playTriggerCheckTwice();
    Resumable playLook(const asn::Look &e, std::optional<asn::Effect> nextEffect = {});
    void playEarlyPlay();
    void playCannotPlay();
    Resumable playDealDamage(const asn::DealDamage &e);
    Resumable playOtherEffect(const asn::OtherEffect &e);
    Resumable playS79_20();

    bool evaluateCondition(const asn::Condition &c);
    bool evaluateConditionIsCard(const asn::ConditionIsCard &c);
    bool evaluateConditionHaveCard(const asn::ConditionHaveCard &c);
    bool evaluateConditionAnd(const asn::ConditionAnd &c);

    void sendLookCard(ServerCard *card);
    std::map<int, ServerCard*> processCommandChooseCard(const CommandChooseCard &cmd);
    Resumable getStagePosition(int &position, const asn::MoveCard &e);
    Resumable moveTopDeck(const asn::MoveCard &e, int toZoneIndex, int toIndex);
};
