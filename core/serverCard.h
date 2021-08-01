#pragma once

#include <string>
#include <tuple>

#include "cardAttribute.pb.h"

#include "abilities/basicTypes.h"
#include "attributeChange.h"
#include "cardInfo.h"

class ServerCardZone;
class ServerPlayer;

class CardBase {
public:
    virtual ~CardBase() = default;

    virtual int level() const = 0;
    virtual int cost() const = 0;
    virtual const std::string& name() const = 0;
    virtual const std::vector<TriggerIcon>& triggers() const = 0;
    virtual const std::vector<std::string>& traits() const = 0;
    virtual CardType type() const = 0;
    virtual bool levelGtPlayerLevel() const = 0;
};

enum StageRow {
    CenterStage,
    BackStage
};

struct AbilityState {
    asn::Ability ability;
    int id = 0; //for communicating with client
    bool permanent = true;
    bool active = false; // for cont
    int duration = 0;
    int activationTimes = 0;
    AbilityState(const asn::Ability &ab, int id_) : ability(ab), id(id_) {}
};

class ServerCard : public CardBase
{
    int mUniqueId;
    std::shared_ptr<CardInfo> mCardInfo;
    ServerCardZone *mZone;
    std::string mCode;
    std::vector<AbilityState> mAbilities;

    std::vector<AttributeChange> mBuffs;
    std::vector<ContAttributeChange> mContBuffs;
    std::vector<BoolAttributeChange> mBoolAttrChanges;
    std::vector<AbilityAsContBuff> mAbilitiesAsContBuffs;

    StageRow mRow;
    int mPosition;
    int mPreviousStagePosition = 0;

    int mPower;
    int mSoul;
    int mLevel;
    asn::State mState = asn::State::Standing;

    bool mInBattle = false;

    bool mTriggerCheckTwice = false;
    bool mCannotPlay = false;
    bool mCannotFrontAttack = false;
    bool mCannotSideAttack = false;

public:
    ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone, int uniqueId);

    void reset();

    int id() const { return mUniqueId; }
    void setPos(int pos);
    int pos() const;
    void setPrevStagePos(int pos) { mPreviousStagePosition = pos; }
    int prevStagePos() const { return mPreviousStagePosition; }
    void setZone(ServerCardZone *zone) { mZone = zone; }
    ServerCardZone* zone() const { return mZone; }
    ServerPlayer* player() const;

    const std::string& code() const { return mCode; }
    const std::string& name() const override { return mCardInfo->name(); }
    int level() const override { return mLevel; }
    int cost() const override { return mCardInfo->cost(); }
    int power() const { return mPower; }
    int soul() const { return mSoul; }
    CardType type() const override { return mCardInfo->type(); }
    char color() const { return mCardInfo->color(); }
    bool isCounter() const { return mCardInfo->isCounter(); }
    asn::State state() const { return mState; }
    void setState(asn::State st) { mState = st; }
    const std::vector<TriggerIcon>& triggers() const override { return mCardInfo->triggers(); }
    const std::vector<std::string>& traits() const override { return mCardInfo->traits(); }
    bool levelGtPlayerLevel() const override;

    bool cannotPlay() const { return mCannotPlay; }
    void setCannotPlay(bool val) { mCannotPlay = val; }
    bool triggerCheckTwice() const { return mTriggerCheckTwice; }
    void setTriggerCheckTwice(bool val) { mTriggerCheckTwice = val; }
    bool inBattle() const { return mInBattle; }
    void setInBattle(bool val) { mInBattle = val; }
    bool cannotFrontAttack() const { return mCannotFrontAttack; }
    void setCannotFrontAttack(bool val) { mCannotFrontAttack = val; }
    bool cannotSideAttack() const { return mCannotSideAttack; }
    void setCannotSideAttack(bool val) { mCannotSideAttack = val; }


    void addAttrBuff(asn::AttributeType attr, int delta, int duration);
    void addBoolAttrChange(BoolAttributeType type, int duration);
    bool addContAttrBuff(ServerCard *card, int abilityId, asn::AttributeType attr, int delta, bool positional = false);
    bool addContBoolAttrChange(ServerCard *card, int abilityId, BoolAttributeType type, bool positional);
    AbilityAsContBuff& addAbilityAsContBuff(ServerCard *card, int abilityId, bool positional, bool &abilityCreated);
    int removeAbilityAsContBuff(ServerCard *card, int abilityId, bool &abilityRemoved);
    int removeAbilityAsPositionalContBuff(bool &abilityRemoved);
    int removeAbilityAsPositionalContBuffBySource(ServerCard *card, bool &abilityRemoved);
    void removeContAttrBuff(ServerCard *card, int abilityId, asn::AttributeType attr);
    void removeContBoolAttrChange(ServerCard *card, int abilityId, BoolAttributeType type);
    void removePositionalContAttrBuffs();
    std::vector<BoolAttributeType> removePositionalContBoolAttrChanges();
    void removeContBuffsBySource(ServerCard *card);
    void removePositionalContAttrBuffsBySource(ServerCard *card);
    std::vector<BoolAttributeType> removePositionalContBoolAttrChangeBySource(ServerCard *card);
    void validateAttrBuffs();
    std::vector<BoolAttributeType> validateBoolAttrChanges();
    bool hasBoolAttrChange(BoolAttributeType type) const;
    std::vector<AbilityState>& abilities() { return mAbilities; }
    int addAbility(const asn::Ability &a, int duration);
    void removeAbility(int id);
    void changeAttr(asn::AttributeType type, int delta);
    void changeBoolAttribute(BoolAttributeType type, bool value);

    int attrByType(asn::AttributeType type) const;
    bool boolAttrByType(BoolAttributeType type) const;
    std::tuple<int, int, int> attributes() const { return { power(), soul(), level() }; }

private:
    int generateAbilitiId() const;
};
