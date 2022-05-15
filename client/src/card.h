#pragma once

#include <string>
#include <vector>

#include <QString>

#include "cardAttribute.pb.h"

#include "abilities.h"
#include "cardInfo.h"
#include "serverCard.h"
#include "abilityModel.h"

class CardZone;

class Card : public CardBase {
    int mId = 0;
    std::string mCode;
    bool mGlow = false;
    bool mSelected = false;
    bool mHighlightedByAbility = false;

    char mColor;
    int mLevel;
    int mCost;
    int mPower;
    int mSoul;
    CardType mType;
    asn::State mState = asn::State::Standing;

    std::unique_ptr<AbilityModel> mAbilityModel;

    std::shared_ptr<CardInfo> mInfo;
    CardZone *mZone;

    std::vector<Card> mMarkers;

    bool mCannotPlay = false;
    bool mCannotFrontAttack = false;
    bool mCannotSideAttack = false;
    bool mCannotBecomeReversed = false;
    bool mCannotMove = false;
    bool mCannotBeChosen = false;

public:
    Card(CardZone *zone) : mZone(zone) {}
    Card(const Card&) = delete;
    Card& operator=(const Card&) = delete;
    Card(Card&&) = default;
    Card& operator=(Card &&other) = default;

    Card(int id, const std::string &code, CardZone *zone);

    void init(int id, const std::string &code);
    void clear();
    bool cardPresent() const { return !mCode.empty(); }

    int id() const { return mId; }
    bool glow() const { return mGlow; }
    bool selected() const { return mSelected; }
    void setGlow(bool glow) { mGlow = glow; }
    void setSelected(bool selected) { mSelected = selected; }
    char color() const { return mColor; }
    int cost() const override { return mCost; }
    bool isCounter() const { return mInfo->isCounter(); }
    int level() const override { return mLevel; }
    void setLevel(int level) { mLevel = level; }
    int power() const override { return mPower; }
    void setPower(int power) { mPower = power; }
    int soul() const { return mSoul; }
    void setSoul(int soul) { mSoul = soul; }
    const std::vector<TriggerIcon>& triggers() const override { return mInfo->triggers(); }
    const std::vector<std::string>& traits() const override { return mInfo->traits(); }
    CardType type() const override { return mType; }
    asn::State state() const override { return mState; }
    void setState(asn::State state) { mState = state; }
    QString qstate() const;
    QString qtype() const;
    QString qcode() const { return QString::fromStdString(mCode); }
    const std::string& code() const { return mCode; }
    const std::string& name() const override { return mInfo->name(); }
    int playersLevel() const override;
    bool cannotPlay() const { return mCannotPlay; }
    void setCannotPlay(bool p) { mCannotPlay = p; }
    bool cannotFrontAttack() const { return mCannotFrontAttack; }
    void setCannotFrontAttack(bool p) { mCannotFrontAttack = p; }
    bool cannotSideAttack() const { return mCannotSideAttack; }
    void setCannotSideAttack(bool p) { mCannotSideAttack = p; }
    bool cannotBecomeReversed() const { return mCannotBecomeReversed; }
    void setCannotBecomeReversed(bool p) { mCannotBecomeReversed = p; }
    bool cannotMove() const { return mCannotMove; }
    void setCannotMove(bool p) { mCannotMove = p; }
    bool cannotBeChosen() const { return mCannotBeChosen; }
    void setCannotBeChosen(bool p) { mCannotBeChosen = p; }
    bool highlightedByAbility() const { return mHighlightedByAbility; }
    void setHighlightedByAbility(bool p) { mHighlightedByAbility = p; }

    QString topMarker() const;
    std::vector<Card>& markers() { return mMarkers; }
    const std::vector<Card>& markers() const { return mMarkers; }
    void addMarker(int id, const std::string &code);

    QString textById(int abilityId) const { return mAbilityModel->textById(abilityId); }
    QString textByIndex(int index) const { return mAbilityModel->textByIndex(index); }
    AbilityModel* textModel() { return mAbilityModel.get(); }
    const AbilityInfo& abilityInfo(int index) const { return mAbilityModel->info(index); }
    int abilityCount() const { return mAbilityModel->count(); }
    const asn::Ability& ability(int index) const { return mAbilityModel->ability(index); }
    const asn::Ability& abilityById(int abilityId) const { return mAbilityModel->abilityById(abilityId); }
    void addAbility(const asn::Ability &a, int id);
    void removeAbilityById(int id);
};
