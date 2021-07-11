#include "serverCard.h"

#include <algorithm>
#include <random>

#include "serverCardZone.h"
#include "serverPlayer.h"

ServerCard::ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone, int uniqueId)
    : mCardInfo(info), mZone(zone) {
    mUniqueId = uniqueId;
    mCode = info->code();
    mPower = info->power();
    mSoul = info->soul();
    mLevel = info->level();

    for (const auto &abBuf: info->abilities())
        mAbilities.emplace_back(decodeAbility(abBuf), static_cast<int>(mAbilities.size()));
}

void ServerCard::reset() {
    mBuffs.clear();
    mContBuffs.clear();
    std::erase_if(mAbilities, [](const AbilityState& ab) { return !ab.permanent; });
    std::for_each(mAbilities.begin(), mAbilities.end(), [](AbilityState& ab) {
        ab.activationTimes = 0;
        ab.active = false;
    });
    mState = StateStanding;

    mPower = mCardInfo->power();
    mSoul = mCardInfo->soul();
    mLevel = mCardInfo->level();

    mTriggerCheckTwice = false;
    mCannotPlay = false;
    mInBattle = false;
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

ServerPlayer* ServerCard::player() const {
    return mZone->player();
}

bool ServerCard::levelGtPlayerLevel() const {
    return mLevel > mZone->player()->level();
}

void ServerCard::addAttrBuff(asn::AttributeType attr, int delta, int duration) {
    mBuffs.emplace_back(attr, delta, duration);
    changeAttr(attr, delta);
}

bool ServerCard::addContAttrBuff(ServerCard *card, int abilityId, asn::AttributeType attr, int delta, bool positional) {
    ContAttributeChange buff(card, abilityId, attr, delta, positional);
    auto it = std::find(mContBuffs.begin(), mContBuffs.end(), buff);
    if (it == mContBuffs.end()) {
        mContBuffs.emplace_back(std::move(buff));
        changeAttr(attr, delta);
        return true;
    }
    if (it->mValue == delta)
        return false;
    changeAttr(attr, delta - it->mValue);
    it->mValue = delta;
    return true;
}

void ServerCard::removeContAttrBuff(ServerCard *card, int abilityId, asn::AttributeType attr) {
    ContAttributeChange buff(card, abilityId, attr, 0);
    auto it = std::find(mContBuffs.begin(), mContBuffs.end(), buff);
    if (it != mContBuffs.end()) {
        changeAttr(attr, -it->mValue);
        mContBuffs.erase(it);
    }
}

void ServerCard::removePositionalContBuffs() {
    for (auto it = mContBuffs.begin(); it != mContBuffs.end();) {
        if (it->mPositional) {
            changeAttr(it->mAttr, -it->mValue);
            it = mContBuffs.erase(it);
        } else {
            ++it;
        }
    }
}

void ServerCard::removeContBuffsBySource(ServerCard *card) {
    for (auto it = mContBuffs.begin(); it != mContBuffs.end();) {
        if (it->mSource == card) {
            changeAttr(it->mAttr, -it->mValue);
            it = mContBuffs.erase(it);
        } else {
            ++it;
        }
    }
}

void ServerCard::removePositionalContBuffsBySource(ServerCard *card) {
    for (auto it = mContBuffs.begin(); it != mContBuffs.end();) {
        if (it->mPositional && it->mSource == card) {
            changeAttr(it->mAttr, -it->mValue);
            it = mContBuffs.erase(it);
        } else {
            ++it;
        }
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
            case asn::AttributeType::Level:
                mLevel -= buff.mValue;
                break;
            default:
                assert(false);
                break;
            }
        }
    }
    std::erase_if(mBuffs, [](const AttributeChange &o){ return o.mDuration <= 0; });
}

int ServerCard::addAbility(const asn::Ability &a, int duration) {
    int id = generateAbilitiId();
    AbilityState s(a, id);
    s.duration = duration;
    s.permanent = false;
    mAbilities.emplace_back(s);
    return id;
}

void ServerCard::changeAttr(asn::AttributeType type, int delta) {
    switch (type) {
    case asn::AttributeType::Power:
        mPower += delta;
        break;
    case asn::AttributeType::Soul:
        mSoul += delta;
        break;
    case asn::AttributeType::Level:
        mLevel += delta;
        break;
    }
}

int ServerCard::attrByType(asn::AttributeType type) const {
    switch (type) {
    case asn::AttributeType::Power:
        return power();
    case asn::AttributeType::Soul:
        return soul();
    case asn::AttributeType::Level:
        return level();
    default:
        assert(false);
        return power();
    }
}

int ServerCard::generateAbilitiId() const {
    while (true) {
        std::mt19937 gen(static_cast<unsigned>(time(nullptr)));
        int id = gen() % 0xFFFF;
        if (std::none_of(mAbilities.begin(), mAbilities.end(), [id](const AbilityState &s){ return s.id == id; }))
            return id;
    }
}
