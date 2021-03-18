#pragma once

#include <vector>

#include "abilities.h"
#include "cardImprint.h"
#include "coroutineTask.h"

class ServerPlayer;

class AbilityPlayer {
    ServerPlayer *mPlayer;
    bool mMandatory = true;
    bool mCanceled = false;
    bool mRevealChosen = false;
    bool mRevert = false; // revert effect of cont ability
    int mAbilityId;
    std::vector<CardImprint> mChosenCards;
    std::vector<CardImprint> mMentionedCards;
    CardImprint mThisCard;
    std::optional<asn::Cost> mCost;

public:
    AbilityPlayer(ServerPlayer *player) : mPlayer(player) {}

    ServerPlayer* owner(asn::Player player) const;

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
    void addMentionedCard(CardImprint &&c) { mMentionedCards.emplace_back(std::move(c)); }
    std::vector<CardImprint>& mentionedCards() { return mMentionedCards; }

    Resumable playAbility(const asn::Ability &a);
    Resumable playAutoAbility(const asn::AutoAbility &a);
    Resumable playEventAbility(const asn::EventAbility &a);

    void playContAbility(const asn::ContAbility &a, bool &active);
    void playContEffect(const asn::Effect &e);
    Resumable playEffect(const asn::Effect &e);
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
    void playMoveWrToDeck(const asn::MoveWrToDeck &e);

    bool evaluateCondition(const asn::Condition &c);
    bool evaluateConditionIsCard(const asn::ConditionIsCard &c);
    bool evaluateConditionHaveCard(const asn::ConditionHaveCard &c);
    bool evaluateConditionAnd(const asn::ConditionAnd &c);

    std::map<int, ServerCard*> processCommandChooseCard(const CommandChooseCard &cmd);
};
