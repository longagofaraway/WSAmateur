#include "card.h"

#include "cardDatabase.h"
#include "cardInfo.h"

Card::Card(const std::string &code) : mCode(code) {
    if (mCode.empty())
        return;

    init(mCode);
}

void Card::init(const std::string &code) {
    mInfo = CardDatabase::get().getCard(code);
    mCode = code;
    mLevel = mInfo->level();
    mCost = mInfo->cost();
    mSoul = mInfo->soul();
    mPower = mInfo->power();
    mColor = mInfo->color();
    mType = mInfo->type();
}

void Card::clear() {
    mInfo.reset();
    mCode = "";
    mLevel = 0;
    mCost = 0;
    mSoul = 0;
    mPower = 0;

    mGlow = false;
    mSelected = false;
    mState = StateStanding;
}

QString Card::qstate() const {
    switch(mState) {
    case StateStanding:
        return QString("Standing");
    case StateRested:
        return QString("Rested");
    case StateReversed:
        return QString("Reversed");
    }
    assert(false);
    return "";
}

QString Card::qtype() const {
    switch(mType) {
    case CardType::Character:
        return QString("Char");
    case CardType::Climax:
        return QString("Climax");
    case CardType::Event:
        return QString("Event");
    }
    assert(false);
    return "";
}
