#include "card.h"

#include <QQmlEngine>

#include "cardDatabase.h"
#include "cardInfo.h"
#include "cardZone.h"
#include "player.h"

Card::Card(const std::string &code, CardZone *zone) : mCode(code), mZone(zone) {
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

    mAbilityModel = std::make_unique<AbilityModel>();
    // we will be returning this pointer via Q_INVOKABLE, so we must set ownership explicitly
    QQmlEngine::setObjectOwnership(mAbilityModel.get(), QQmlEngine::CppOwnership);
    for (const auto &abBuf: mInfo->abilities())
        mAbilityModel->addAbility(decodeAbility(abBuf), mAbilityModel->count(), mType);
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
    mAbilityModel.reset();
}

QString Card::qstate() const {
    switch(mState) {
    case StateStanding:
        return QString("Standing");
    case StateRested:
        return QString("Rested");
    case StateReversed:
        return QString("Reversed");
    default:
        break;
    }
    assert(false);
    return "";
}

QString Card::qtype() const {
    switch(mType) {
    case CardType::Char:
        return QString("Char");
    case CardType::Climax:
        return QString("Climax");
    case CardType::Event:
        return QString("Event");
    default:
        break;
    }
    return "";
}

bool Card::levelGtPlayerLevel() const {
    return mLevel > mZone->player()->level();
}

void Card::addAbility(const asn::Ability &a, int id) {
    mAbilityModel->addAbility(a, id, mType, false);
}

void Card::removeAbilityById(int id) {
    mAbilityModel->removeAbilityById(id);
}
