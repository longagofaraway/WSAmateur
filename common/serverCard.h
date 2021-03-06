#pragma once

#include <string>

#include "cardAttribute.pb.h"

#include "abilities/basicTypes.h"
#include "attributeChange.h"
#include "cardInfo.h"

class ServerCardZone;

class CardBase {
public:
    virtual ~CardBase() = default;

    virtual int level() const = 0;
    virtual int cost() const = 0;
    virtual const std::string& name() const = 0;
    virtual const std::vector<TriggerIcon>& triggers() const = 0;
    virtual const std::vector<std::string>& traits() const = 0;
    virtual CardType type() const = 0;
};

enum StageRow {
    CenterStage,
    BackStage
};

struct AbilityState {
    asn::Ability ability;
    bool permanent = true;
    bool active = false; // for cont
    int duration = 0;
    int activationTimes = 0;
    AbilityState(const asn::Ability &ab) : ability(ab) {}
};

class ServerCard : public CardBase
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
    const std::string& name() const override { return mCardInfo->name(); }
    int level() const override { return mCardInfo->level(); }
    int cost() const override { return mCardInfo->cost(); }
    int power() const { return mPower; }
    int soul() const { return mSoul; }
    CardType type() const override { return mCardInfo->type(); }
    char color() const { return mCardInfo->color(); }
    CardState state() const { return mState; }
    void setState(CardState state) { mState = state; }
    const std::vector<TriggerIcon>& triggers() const override { return mCardInfo->triggers(); }
    const std::vector<std::string>& traits() const override { return mCardInfo->traits(); }
    void addAttrBuff(asn::AttributeType attr, int delta, int duration);
    void validateBuffs();
    std::vector<AbilityState>& abilities() { return mAbilities; }
    void addAbility(const asn::Ability &a, int duration);
    void changeAttr(asn::AttributeType type, int delta);
};
