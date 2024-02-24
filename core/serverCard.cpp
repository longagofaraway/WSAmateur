#include "serverCard.h"

#include <algorithm>
#include <random>

#include "serverCardZone.h"
#include "serverPlayer.h"

ServerCard::ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone, int uniqueId)
    : mCardInfo(info), mZone(zone), mBuffManager(this) {
    mUniqueId = uniqueId;
    mCode = info->code();
    mPower = info->power();
    mSoul = info->soul();
    mLevel = info->level();
    mTriggerIcons = info->triggers();

    for (const auto &abBuf: info->abilities())
        mAbilities.emplace_back(decodeAbility(abBuf), static_cast<int>(mAbilities.size()));
}

// reset is called when changing zones
void ServerCard::reset() {
    mBuffManager.reset();
    std::erase_if(mAbilities, [](const AbilityState& ab) { return !ab.permanent; });
    std::for_each(mAbilities.begin(), mAbilities.end(), [](AbilityState& ab) {
        ab.activationTimes = 0;
        ab.active = false;
    });
    mMarkers.clear();
    mState = asn::State::Standing;
    mFaceOrientation = asn::FaceOrientation::FaceUp;

    mPower = mCardInfo->power();
    mSoul = mCardInfo->soul();
    mLevel = mCardInfo->level();
    // do not reset trigger icons

    mTriggerCheckTwice = false;
    mCannotPlay = false;
    mCannotFrontAttack = false;
    mCannotSideAttack = false;
    mCannotBecomeReversed = false;
    mSideAttackWithoutPenalty = false;
    mCannotStand = false;
    mCannotMove = false;
    mCannotBeChosen = false;
    mCanPlayWithoutColorRequirement = false;
    mInBattle = false;
    mFirstTurn = false;
    mIsMarker = false;
}

void ServerCard::setPos(int pos) {
    mPosition = pos;
}

int ServerCard::pos() const {
    return mPosition;
}

ServerPlayer* ServerCard::player() const {
    return mZone->player();
}

void ServerCard::addTriggerIcon(asn::TriggerIcon icon) {
    mTriggerIcons.push_back(icon);
}

void ServerCard::removeTriggerIcon(asn::TriggerIcon icon) {
    auto it = std::find(mTriggerIcons.begin(), mTriggerIcons.end(), icon);
    if (it != mTriggerIcons.end())
        mTriggerIcons.erase(it);
}

int ServerCard::playersLevel() const {
    return mZone->player()->level();
}

ServerCard* ServerCard::addMarker(std::unique_ptr<ServerCard> &&card) {
    card->setPos(mPosition);
    card->setIsMarker(true);
    card->setZone(zone());
    return mMarkers.emplace_back(std::move(card)).get();
}

std::unique_ptr<ServerCard> ServerCard::takeTopMarker() {
    if (mMarkers.empty())
        return {};

    auto card = std::move(mMarkers.back());
    mMarkers.pop_back();
    return card;
}

std::unique_ptr<ServerCard> ServerCard::takeMarker(int index) {
    if (static_cast<size_t>(index) >= mMarkers.size())
        return {};

    auto card = std::move(mMarkers[index]);
    mMarkers.erase(mMarkers.begin() + index);
    return card;
}


int ServerCard::addAbility(const asn::Ability &a) {
    int id = generateAbilitiId();
    AbilityState s(a, id);
    s.permanent = false;
    mAbilities.emplace_back(s);
    return id;
}

void ServerCard::removeAbility(int id) {
    std::erase_if(mAbilities, [id](auto &ability) { return ability.id == id; });
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
    default:
        assert(false);
    }
}

void ServerCard::changeBoolAttribute(BoolAttributeType type, bool value) {
    switch (type) {
    case BoolAttributeType::CannotFrontAttack:
        mCannotFrontAttack = value;
        break;
    case BoolAttributeType::CannotSideAttack:
        mCannotSideAttack = value;
        break;
    case BoolAttributeType::CannotBecomeReversed:
        mCannotBecomeReversed = value;
        break;
    case BoolAttributeType::CannotMove:
        mCannotMove = value;
        break;
    case BoolAttributeType::SideAttackWithoutPenalty:
        mSideAttackWithoutPenalty = value;
        break;
    case BoolAttributeType::CannotStand:
        mCannotStand = value;
        break;
    case BoolAttributeType::CannotBeChosen:
        mCannotBeChosen = value;
        break;
    case BoolAttributeType::CanPlayWithoutColorRequirement:
        mCanPlayWithoutColorRequirement = value;
        break;
    default:
        assert(false);
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

bool ServerCard::boolAttrByType(BoolAttributeType type) const {
    switch (type) {
    case BoolAttributeType::CannotFrontAttack:
        return cannotFrontAttack();
    case BoolAttributeType::CannotSideAttack:
        return cannotSideAttack();
    case BoolAttributeType::CannotBecomeReversed:
        return cannotBecomeReversed();
    case BoolAttributeType::CannotMove:
        return cannotMove();
    case BoolAttributeType::SideAttackWithoutPenalty:
        return sideAttackWithoutPenalty();
    case BoolAttributeType::CannotStand:
        return cannotStand();
    case BoolAttributeType::CannotBeChosen:
        return cannotBeChosen();
    case BoolAttributeType::CanPlayWithoutColorRequirement:
        return canPlayWithoutColorRequirement();
    default:
        assert(false);
        return false;
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

const std::set<std::string> ServerCard::traits() const {
    const auto &traits = mCardInfo->traits();
    std::set<std::string> resultTraits{traits.begin(), traits.end()};
    for (const auto traitModification: mBuffManager.traitChanges()) {
        if (traitModification->type == asn::TraitModificationType::TraitGain) {
            resultTraits.insert(traitModification->trait);
        } else {
            resultTraits.erase(traitModification->trait);
        }
    }
    return resultTraits;
}
