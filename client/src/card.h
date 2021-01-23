#pragma once

#include <string>

#include <QString>

#include "cardAttribute.pb.h"

#include "cardInfo.h"

class Card {
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

    std::shared_ptr<CardInfo> mInfo;

public:
    Card() = default;
    Card(const Card&) = delete;
    Card(Card&&) = default;
    Card& operator=(Card &&other) = default;

    Card(const std::string &code);

    void init(const std::string &code);
    void clear();
    bool cardPresent() const { return !mCode.empty(); }

    bool glow() const { return mGlow; }
    bool selected() const { return mSelected; }
    void setGlow(bool glow) { mGlow = glow; }
    void setSelected(bool selected) { mSelected = selected; }
    char color() const { return mColor; }
    int cost() const { return mCost; }
    int level() const { return mLevel; }
    void setLevel(int level) { mLevel = level; }
    int power() const { return mPower; }
    void setPower(int power) { mPower = power; }
    int soul() const { return mSoul; }
    void setSoul(int soul) { mSoul = soul; }
    CardType type() const { return mType; }
    CardState state() const { return mState; }
    void setState(CardState state) { mState = state; }
    QString qstate() const;
    QString qtype() const;
    QString qcode() const { return QString::fromStdString(mCode); }
};
