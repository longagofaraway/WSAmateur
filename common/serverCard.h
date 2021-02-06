#pragma once

#include <string>

#include "cardAttribute.pb.h"

#include "abilities/basicTypes.h"
#include "attributeChange.h"
#include "cardInfo.h"

class ServerCardZone;

enum StageRow {
    CenterStage,
    BackStage
};

struct AbilityState {
    asn::Ability ability;
    bool permanent = true;
    int duration = 0;
    int activationTimes = 0;
    AbilityState(asn::Ability ab) : ability(ab) {}
};

class ServerCard
{
    std::shared_ptr<CardInfo> mCardInfo;
    ServerCardZone *mZone;
    std::string mCode;
    std::vector<AttributeChange> mBuffs;
    std::vector<AbilityState> mAbilities;

    // stage position
    StageRow mRow;
    int mPosition;

    int mPower;
    int mSoul;
    int mLevel;
    CardState mState = StateStanding;

public:
    ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone);
    ServerCard(int pos, ServerCardZone *zone);

    void reset();

    void setPos(int pos);
    int pos() const;
    void setZone(ServerCardZone *zone) { mZone = zone; }
    ServerCardZone* zone() const { return mZone; }
    const std::string& code() { return mCode; }
    int level() const { return mCardInfo->level(); }
    int cost() const { return mCardInfo->cost(); }
    int power() const { return mPower; }
    int soul() const { return mSoul; }
    CardType type() const { return mCardInfo->type(); }
    char color() const { return mCardInfo->color(); }
    CardState state() const { return mState; }
    void setState(CardState state) { mState = state; }
    const std::vector<TriggerIcon>& triggers() { return mCardInfo->triggers(); }
    void addAttrBuff(CardAttribute attr, int delta, int duration);
    void validateBuffs();
};
