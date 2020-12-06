#pragma once

#include <string>
#include <vector>

enum Trigger {
    Soul,
    Door
};

class CardInfo
{
    int mLevel;
    int mPower;
    int mCost;
    int mSoul;
    char mColor;
    std::string mCode;
    std::string mName;
    std::string mType;
    std::vector<std::string> mTraits;
    std::vector<Trigger> mTriggers;
    //std::vector<Ability> mAbilities;

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
    const std::string& code() const { return mCode; }
    void setCode(const std::string &code) { mCode = code; }
    const std::string& name() const { return mName; }
    void setName(const std::string &name) { mName = name; }
    const std::string& type() const { return mType; }
    void setType(const std::string &type) { mType = type; }
    const std::vector<std::string>& traits() const { return mTraits; }
    void setTraits(const std::vector<std::string> &traits) { mTraits = traits; }
    const std::vector<Trigger>& triggers() const { return mTriggers; }
    void setTriggers(const std::vector<Trigger> &triggers) { mTriggers = triggers; }
};
