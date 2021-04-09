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
    std::string mCode;
    bool mGlow = false;
    bool mSelected = false;

    char mColor;
    int mLevel;
    int mCost;
    int mPower;
    int mSoul;
    CardType mType;
    CardState mState = StateStanding;

    std::unique_ptr<AbilityModel> mAbilityModel;

    std::shared_ptr<CardInfo> mInfo;
    CardZone *mZone;

    bool mCannotPlay = false;

public:
    Card(CardZone *zone) : mZone(zone) {}
    Card(const Card&) = delete;
    Card& operator=(const Card&) = delete;
    Card(Card&&) = default;
    Card& operator=(Card &&other) = default;

    Card(const std::string &code, CardZone *zone);

    void init(const std::string &code);
    void clear();
    bool cardPresent() const { return !mCode.empty(); }

    bool glow() const { return mGlow; }
    bool selected() const { return mSelected; }
    void setGlow(bool glow) { mGlow = glow; }
    void setSelected(bool selected) { mSelected = selected; }
    char color() const { return mColor; }
    int cost() const override { return mCost; }
    bool isCounter() const { return mInfo->isCounter(); }
    int level() const override { return mLevel; }
    void setLevel(int level) { mLevel = level; }
    int power() const { return mPower; }
    void setPower(int power) { mPower = power; }
    int soul() const { return mSoul; }
    void setSoul(int soul) { mSoul = soul; }
    const std::vector<TriggerIcon>& triggers() const override { return mInfo->triggers(); }
    const std::vector<std::string>& traits() const override { return mInfo->traits(); }
    CardType type() const override { return mType; }
    CardState state() const { return mState; }
    void setState(CardState state) { mState = state; }
    QString qstate() const;
    QString qtype() const;
    QString qcode() const { return QString::fromStdString(mCode); }
    const std::string& code() const { return mCode; }
    const std::string& name() const override { return mInfo->name(); }
    bool levelGtPlayerLevel() const;
    bool cannotPlay() const { return mCannotPlay; }
    void setCannotPlay(bool p) { mCannotPlay = p; }

    QString text(int abilityId) const { return mAbilityModel->text(abilityId); }
    AbilityModel* textModel() { return mAbilityModel.get(); }
    int abilityCount() const { return mAbilityModel->count(); }
    const asn::Ability& ability(int abilityId) const { return mAbilityModel->ability(abilityId); }
    void addAbility(const asn::Ability &a);
    void removeAbility(int id);
};
