#include "serverCard.h"

ServerCard::ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone)
    : mCardInfo(info), mZone(zone) {
    mCode = info->code();
    mPower = info->power();
    mSoul = info->soul();
    mLevel = info->level();

    for (const auto &abBuf: info->abilities())
        mAbilities.emplace_back(decodeAbility(abBuf));
}

ServerCard::ServerCard(int pos, ServerCardZone *zone)
    : mZone(zone) {
    setPos(pos);
}

void ServerCard::reset() {
    mBuffs.clear();
    std::erase_if(mAbilities, [](const AbilityState& ab) { return !ab.permanent; });
    std::for_each(mAbilities.begin(), mAbilities.end(), [](AbilityState& ab) { ab.activationTimes = 0; });
    mState = StateStanding;

    mPower = mCardInfo->power();
    mSoul = mCardInfo->soul();
    mLevel = mCardInfo->level();
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

void ServerCard::addAttrBuff(asn::AttributeType attr, int delta, int duration) {
    mBuffs.emplace_back(attr, delta, duration);
    switch (attr) {
    case asn::AttributeType::Soul:
        mSoul += delta;
        break;
    case asn::AttributeType::Power:
        mPower += delta;
        break;
    default:
        assert(false);
        break;
    }
}

void ServerCard::validateBuffs() {
    for (auto &buff: mBuffs) {
        if (--buff.mDuration == 0) {
            switch (buff.mAttr) {
            case asn::AttributeType::Soul:
                mSoul -= buff.mValue;
                break;
            case asn::AttributeType::Power:
                mPower -= buff.mValue;
                break;
            default:
                assert(false);
                break;
            }
        }
    }
    std::erase_if(mBuffs, [](const AttributeChange &o){ return o.mDuration <= 0; });
}
