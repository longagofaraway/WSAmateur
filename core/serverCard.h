#pragma once

#include <string>
#include <tuple>

#include "cardAttribute.pb.h"

#include "abilities/basicTypes.h"
#include "attributeChange.h"
#include "cardBuffManager.h"
#include "cardInfo.h"

class ServerCardZone;
class ServerPlayer;

class CardBase {
public:
    virtual ~CardBase() = default;

    virtual int power() const = 0;
    virtual int level() const = 0;
    virtual int cost() const = 0;
    virtual const std::string& name() const = 0;
    virtual const std::vector<TriggerIcon>& triggers() const = 0;
    virtual const std::vector<std::string>& traits() const = 0;
    virtual CardType type() const = 0;
    virtual int playersLevel() const = 0;
};

enum StageRow {
    CenterStage,
    BackStage
};

struct AbilityState {
    asn::Ability ability;
    int id = 0;
    bool permanent = true;
    bool active = false; // for cont
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

    std::vector<std::unique_ptr<ServerCard>> mMarkers;

    StageRow mRow;
    int mPosition;
    int mPreviousStagePosition = 0;

    int mPower;
    int mSoul;
    int mLevel;
    asn::State mState = asn::State::Standing;
    asn::FaceOrientation mFaceOrientation = asn::FaceOrientation::FaceUp;

    bool mInBattle = false;

    bool mTriggerCheckTwice = false;
    bool mCannotPlay = false;
    bool mCannotFrontAttack = false;
    bool mCannotSideAttack = false;
    bool mCannotBecomeReversed = false;
    bool mCannotMove = false;
    bool mSideAttackWithoutPenalty = false;
    bool mCannotStand = false;
    bool mFirstTurn = false;

    CardBuffManager mBuffManager;

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
    int power() const override { return mPower; }
    int soul() const { return mSoul; }
    CardType type() const override { return mCardInfo->type(); }
    char color() const { return mCardInfo->color(); }
    bool isCounter() const { return mCardInfo->isCounter(); }
    asn::State state() const { return mState; }
    void setState(asn::State st) { mState = st; }
    const std::vector<TriggerIcon>& triggers() const override { return mCardInfo->triggers(); }
    const std::vector<std::string>& traits() const override { return mCardInfo->traits(); }
    int playersLevel() const override;
    asn::FaceOrientation faceOrientation() const { return mFaceOrientation; }

    bool hasMarkers() const { return mMarkers.size(); }
    std::vector<std::unique_ptr<ServerCard>>& markers() { return mMarkers; }
    ServerCard* addMarker(std::unique_ptr<ServerCard> &&card);
    std::unique_ptr<ServerCard> takeTopMarker();

    bool cannotPlay() const { return mCannotPlay; }
    void setCannotPlay(bool val) { mCannotPlay = val; }
    bool triggerCheckTwice() const { return mTriggerCheckTwice; }
    void setTriggerCheckTwice(bool val) { mTriggerCheckTwice = val; }
    bool inBattle() const { return mInBattle; }
    void setInBattle(bool val) { mInBattle = val; }
    bool cannotFrontAttack() const { return mCannotFrontAttack; }
    bool cannotSideAttack() const { return mCannotSideAttack; }
    bool cannotBecomeReversed() const { return mCannotBecomeReversed; }
    bool cannotMove() const { return mCannotMove; }
    bool cannotStand() const { return mCannotStand; }
    bool sideAttackWithoutPenalty() const { return mSideAttackWithoutPenalty; }
    void setFirstTurn(bool val) { mFirstTurn = val; }
    bool isFirstTurn() const { return mFirstTurn; }

    CardBuffManager* buffManager() { return &mBuffManager; }

    std::vector<AbilityState>& abilities() { return mAbilities; }
    int addAbility(const asn::Ability &a);
    void removeAbility(int id);
    void changeAttr(asn::AttributeType type, int delta);
    void changeBoolAttribute(BoolAttributeType type, bool value);

    int attrByType(asn::AttributeType type) const;
    bool boolAttrByType(BoolAttributeType type) const;
    std::tuple<int, int, int> attributes() const { return { power(), soul(), level() }; }

private:
    int generateAbilitiId() const;
};
