#pragma once

#include <string>
#include <vector>

#include "abilities.h"

using asn::TriggerIcon;
using asn::CardType;
using asn::Color;

class CardInfo
{
    int mLevel;
    int mPower;
    int mCost;
    int mSoul;
    Color mColor;
    bool mCounter;
    CardType mType;
    std::string mCode;
    std::string mName;
    std::vector<std::string> mTraits;
    std::vector<TriggerIcon> mTriggers;
    std::vector<std::vector<uint8_t>> mAbilities;
    std::vector<std::string> mReferences;

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
    Color color() const { return mColor; }
    void setColor(Color color) { mColor = color; }
    CardType type() const { return mType; }
    void setType(CardType type) { mType = type; }
    bool isCounter() const { return mCounter; }
    void setCounter(bool counter) { mCounter = counter; }
    const std::string& code() const { return mCode; }
    void setCode(const std::string &code) { mCode = code; }
    const std::string& name() const { return mName; }
    void setName(const std::string &name) { mName = name; }
    const std::vector<std::string>& traits() const { return mTraits; }
    void setTraits(const std::vector<std::string> &traits) { mTraits = traits; }
    const std::vector<TriggerIcon>& triggers() const { return mTriggers; }
    void setTriggers(const std::vector<TriggerIcon> &triggers) { mTriggers = triggers; }
    const std::vector<std::vector<uint8_t>>& abilities() const { return mAbilities; }
    void addAbility(const std::vector<uint8_t> &ability) { mAbilities.push_back(ability); }
    const std::vector<std::string>& references() const { return mReferences; }
    void setReferences(const std::vector<std::string> &ref) { mReferences = ref; }
};
