#include "card.h"

#include "cardDatabase.h"
#include "cardInfo.h"

Card::Card(std::string code) : mCode(code) {
    if (mCode.empty())
        return;

    mInfo = CardDatabase::get().getCard(code);
    mLevel = mInfo->level();
    mCost = mInfo->cost();
    mSoul = mInfo->soul();
    mPower = mInfo->power();
}
