#include "serverCard.h"

ServerCard::ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone)
    : mCardInfo(info), mZone(zone) {
    mCode = info->code();
    mPower = info->power();
    mSoul = info->soul();
}

ServerCard::ServerCard(int pos, ServerCardZone *zone)
    : mZone(zone) {
    setPos(pos);
}

void ServerCard::reset() {
    mBuffs.clear();
    mState = StateStanding;

    mPower = mCardInfo->power();
    mSoul = mCardInfo->soul();
}

void ServerCard::setPos(int pos) {
    mPosition = pos;
    if (pos < 3)
        mRow = CenterStage;
    else
        mRow = BackStage;
}

int ServerCard::pos() const {
    return mPosition;
}

void ServerCard::addAttrBuff(CardAttribute attr, int delta, int duration) {
    mBuffs.emplace_back(attr, delta, duration);
    switch (attr) {
    case AttrSoul:
        mSoul += delta;
        break;
    case AttrPower:
        mPower += delta;
        break;
    }
}

void ServerCard::validateBuffs() {
    for (auto &buff: mBuffs) {
        if (--buff.mDuration == 0) {
            switch (buff.mAttr) {
            case AttrSoul:
                mSoul -= buff.mValue;
                break;
            case AttrPower:
                mPower -= buff.mValue;
                break;
            }
        }
    }
    mBuffs.erase(std::remove_if(mBuffs.begin(), mBuffs.end(),
                                [](const AttributeChange &o){ return o.mDuration <= 0; }),
                 mBuffs.end());
}
