#pragma once

#include <string>
#include <vector>

enum Trigger {
    Soul,
    Door
};

enum class CardType {
    Character,
    Climax,
    Event
};

enum class CardState {
    Standing,
    Rested,
    Reversed
};

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
    void setColor(char color) {
        mColor = color;
    }
    CardType type() const { return mType; }
    void setType(CardType type) { mType = type; }
    const std::string& code() const { return mCode; }
    void setCode(const std::string &code) { mCode = code; }
    const std::string& name() const { return mName; }
    void setName(const std::string &name) { mName = name; }
    const std::vector<std::string>& traits() const { return mTraits; }
    void setTraits(const std::vector<std::string> &traits) { mTraits = traits; }
    const std::vector<Trigger>& triggers() const { return mTriggers; }
    void setTriggers(const std::vector<Trigger> &triggers) { mTriggers = triggers; }
};
