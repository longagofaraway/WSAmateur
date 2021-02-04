#pragma once

#include <string>
#include <vector>

#include "abilities.h"

/*enum class Trigger {
    Soul,
    Door
};*/

using asn::TriggerIcon;
using asn::CardType;

class CardInfo
{
    int mLevel;
    int mPower;
    int mCost;
    int mSoul;
    char mColor;
    CardType mType;
    std::string mCode;
    std::string mName;
    std::vector<std::string> mTraits;
    std::vector<TriggerIcon> mTriggers;
    std::vector<asn::Ability> mAbilities;
    std::vector<std::string> mText;

public:
    CardInfo();

    int level() const { return mLevel; }
    void setLevel(int level) { mLevel = level; }
    int power() const { return mPower; }
    void setPower(int power) { mPower = power; }
    int cost() const { return mCost; }
    void setCost(int cost) { mCost = cost; }
    int soul() const { return mSoul; }
    void setSoul(int soul) { mSoul = soul; }
    char color() const { return mColor; }
    void setColor(char color) { mColor = color; }
    asn::CardType type() const { return mType; }
    void setType(CardType type) { mType = type; }
    const std::string& code() const { return mCode; }
    void setCode(const std::string &code) { mCode = code; }
    const std::string& name() const { return mName; }
    void setName(const std::string &name) { mName = name; }
    const std::vector<std::string>& traits() const { return mTraits; }
    void setTraits(const std::vector<std::string> &traits) { mTraits = traits; }
    const std::vector<TriggerIcon>& triggers() const { return mTriggers; }
    void setTriggers(const std::vector<TriggerIcon> &triggers) { mTriggers = triggers; }
};
