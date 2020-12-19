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
