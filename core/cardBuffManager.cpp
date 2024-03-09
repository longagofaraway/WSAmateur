#include "cardBuffManager.h"

#include <algorithm>
#include <random>

#include <QDebug>

#include "abilityEvents.pb.h"
#include "gameEvent.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverCard.h"
#include "serverPlayer.h"

namespace {
int generateId() {
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                   std::chrono::system_clock::now().time_since_epoch()).count();
    std::mt19937 gen(milliseconds);
    int id = gen() % 0xFFFF;
    return id;
}
}

void CardBuffManager::reset() {
    mBuffs.clear();
    mContBuffs.clear();
    mAbilityBuffs.clear();
    mBoolAttrChanges.clear();

    // trigger icon works in all zones
    std::erase_if(cardBuffs, [](const std::unique_ptr<Buff> &buff) {
        return buff->buffType != BuffType::TriggerIcon;
    });
}

void CardBuffManager::addBuff(const Buff &buff) {
    if (buff.duration > 0) {
        const auto& newBuff = cardBuffs.emplace_back(buff.clone());
        newBuff->apply(mCard);
        return;
    }
    auto it = std::find(cardBuffs.begin(), cardBuffs.end(), buff);
    if (it == cardBuffs.end()) {
        const auto& newBuff = cardBuffs.emplace_back(buff.clone());
        newBuff->apply(mCard);
        return;
    }
    it->get()->update(buff);
}

void CardBuffManager::removeContBuff(const Buff &buff) {
    auto it = std::find(cardBuffs.begin(), cardBuffs.end(), buff);
    if (it == cardBuffs.end())
        return;

    removeBuff(it);
}

void CardBuffManager::sendAttrChange(asn::AttributeType attr) {
    EventSetCardAttr event;
    event.set_card_pos(mCard->pos());
    event.set_zone(mCard->zone()->name());
    event.set_attr(attrTypeToProto(attr));
    event.set_value(mCard->attrByType(attr));
    if (mCard->zone()->type() == ZoneType::PrivateZone)
        mCard->player()->sendGameEvent(event);
    else
        mCard->player()->sendToBoth(event);
}

void CardBuffManager::sendChangedAttrs(std::tuple<int, int, int> oldAttrs) {
    auto [oldPower, oldSoul, oldLevel] = oldAttrs;
    if (mCard->power() != oldPower)
        sendAttrChange(asn::AttributeType::Power);
    if (mCard->soul() != oldSoul)
        sendAttrChange(asn::AttributeType::Soul);
    if (mCard->level() != oldLevel)
        sendAttrChange(asn::AttributeType::Level);
}

void CardBuffManager::sendBoolAttrChange(BoolAttributeType type, bool value) {
    EventSetCardBoolAttr event;
    event.set_card_pos(mCard->pos());
    event.set_zone(mCard->zone()->name());
    event.set_attr(getProtoBoolAttrType(type));
    event.set_value(value);
    mCard->player()->sendToBoth(event);
}

void CardBuffManager::addAttributeBuff(asn::AttributeType attr, int delta, int duration) {
    mBuffs.emplace_back(attr, delta, duration);
    mCard->changeAttr(attr, delta);
    if (mCard->zone()->name() == "stage" && mCard->power() <= 0)
        mCard->player()->triggerRuleAction(RuleAction::InsufficientPower, mCard);

    sendAttrChange(attr);
}

void CardBuffManager::addBoolAttrChange(BoolAttributeType type, int duration) {
    mBoolAttrChanges.emplace_back(type, duration);
    if (mCard->boolAttrByType(type))
        return;

    mCard->changeBoolAttribute(type, true);
    sendBoolAttrChange(type, true);
}

void CardBuffManager::addContAttributeBuff(ServerCard *source, int abilityId, asn::AttributeType attr,
                                           int delta, bool positional) {
    ContAttributeChange buff(source, abilityId, attr, delta, positional);
    auto it = std::find(mContBuffs.begin(), mContBuffs.end(), buff);
    if (it == mContBuffs.end()) {
        mContBuffs.emplace_back(std::move(buff));
        mCard->changeAttr(attr, delta);
    } else {
        if (it->value == delta)
            return;
        mCard->changeAttr(attr, delta - it->value);
        it->value = delta;
    }
    if (mCard->zone()->name() == "stage" && mCard->power() <= 0)
        mCard->player()->triggerRuleAction(RuleAction::InsufficientPower, mCard);

    sendAttrChange(attr);
}

void CardBuffManager::addContBoolAttrChange(ServerCard *source, int abilityId, BoolAttributeType type, bool positional) {
    BoolAttributeChange change(source, abilityId, type, positional);
    auto it = std::find(mBoolAttrChanges.begin(), mBoolAttrChanges.end(), change);
    if (it != mBoolAttrChanges.end())
        return;

    mBoolAttrChanges.emplace_back(std::move(change));
    if (mCard->boolAttrByType(type))
        return;

    mCard->changeBoolAttribute(type, true);
    sendBoolAttrChange(type, true);
}

void CardBuffManager::removeContAttributeBuff(ServerCard *source, int abilityId, asn::AttributeType attr) {
    ContAttributeChange buff(source, abilityId, attr, 0);
    auto it = std::find(mContBuffs.begin(), mContBuffs.end(), buff);
    if (it == mContBuffs.end())
        return;

    mCard->changeAttr(attr, -it->value);
    mContBuffs.erase(it);

    if (mCard->zone()->name() == "stage" && mCard->power() <= 0)
        mCard->player()->triggerRuleAction(RuleAction::InsufficientPower, mCard);

    sendAttrChange(attr);
}

void CardBuffManager::removeContBoolAttrChange(ServerCard *source, int abilityId, BoolAttributeType type) {
    BoolAttributeChange change(source, abilityId, type);
    auto it = std::find(mBoolAttrChanges.begin(), mBoolAttrChanges.end(), change);
    if (it == mBoolAttrChanges.end())
        return;

    mBoolAttrChanges.erase(it);
    if (!hasBoolAttrChange(type)) {
        mCard->changeBoolAttribute(type, false);
        sendBoolAttrChange(type, false);
    }
}

int CardBuffManager::addAbility(const asn::Ability &a) {
    int newId = mCard->addAbility(a);

    EventAbilityGain ev;
    ev.set_zone(mCard->zone()->name());
    ev.set_card_pos(mCard->pos());
    ev.set_ability_id(newId);

    std::vector<uint8_t> abilityBuf = encodeAbility(a);
    ev.set_ability(abilityBuf.data(), abilityBuf.size());
    mCard->player()->sendToBoth(ev);

    if (a.type == asn::AbilityType::Cont)
        mCard->player()->playContAbilities(mCard);

    return newId;
}

void CardBuffManager::addAbilityBuff(const asn::Ability &a, int duration) {
    int newId = addAbility(a);
    mAbilityBuffs.emplace_back(newId, duration);
}

void CardBuffManager::addAbilityAsContBuff(ServerCard *source, int sourceAbilityId,
                                           const asn::Ability &ability, bool positional) {
    AbilityBuff buff(source, sourceAbilityId, positional);
    auto it = std::find(mAbilityBuffs.begin(), mAbilityBuffs.end(), buff);
    if (it != mAbilityBuffs.end())
        return;

    auto &addedBuff = mAbilityBuffs.emplace_back(std::move(buff));
    addedBuff.abilityId = addAbility(ability);
}

void CardBuffManager::removeAbility(int abilityId) {
    for (const auto &it: mCard->abilities()) {
        if (it.id != abilityId)
            continue;

        if (it.active && it.ability.type == asn::AbilityType::Cont) {
            AbilityPlayer a(mCard->player());
            a.setThisCard(CardImprint(mCard->zone()->name(), mCard));
            a.setAbilityId(it.id);
            a.revertContAbility(std::get<asn::ContAbility>(it.ability.ability));
        }

        mCard->removeAbility(abilityId);

        EventRemoveAbility event;
        event.set_card_pos(mCard->pos());
        event.set_zone(mCard->zone()->name());
        event.set_ability_id(abilityId);
        mCard->player()->sendToBoth(event);

        break;
    }
}

void CardBuffManager::removeAbilityAsContBuff(ServerCard *source, int sourceAbilityId) {
    AbilityBuff buff(source, sourceAbilityId, false);
    auto it = std::find(mAbilityBuffs.begin(), mAbilityBuffs.end(), buff);
    if (it == mAbilityBuffs.end())
        return;

    removeAbility(it->abilityId);
    mAbilityBuffs.erase(it);
}

void CardBuffManager::removePositionalContBuffs() {
    // do not send attr changes intentionally
    removePositionalContAttrBuffs();
    removePositionalContBoolAttrChanges();
    removeAbilityAsPositionalContBuff();

    for (auto it = cardBuffs.begin(); it != cardBuffs.end();) {
        if ((*it)->positional)
            it = removeBuff(it);
        else
            ++it;
    }

    if (mCard->zone()->name() == "stage" && mCard->power() <= 0)
        mCard->player()->triggerRuleAction(RuleAction::InsufficientPower, mCard);
}

void CardBuffManager::removePositionalContAttrBuffs() {
    for (auto it = mContBuffs.begin(); it != mContBuffs.end();) {
        if (it->positional) {
            mCard->changeAttr(it->attr, -it->value);
            it = mContBuffs.erase(it);
        } else {
            ++it;
        }
    }
}

void CardBuffManager::removePositionalContBoolAttrChanges() {
    for (auto it = mBoolAttrChanges.begin(); it != mBoolAttrChanges.end();) {
        if (it->positional && it->duration == 0) {
            auto type = it->type;
            it = mBoolAttrChanges.erase(it);
            if (!hasBoolAttrChange(type)) {
                mCard->changeBoolAttribute(type, false);
                sendBoolAttrChange(type, false);
            }
        } else {
            ++it;
        }
    }
}

void CardBuffManager::removeAbilityAsPositionalContBuff() {
    for (auto it = mAbilityBuffs.begin(); it != mAbilityBuffs.end();) {
        if (it->positional && it->duration == 0) {
            removeAbility(it->abilityId);
            it = mAbilityBuffs.erase(it);
        } else {
            ++it;
        }
    }
}

void CardBuffManager::removePositionalContBuffsBySource(ServerCard *source) {
    auto oldAttrs = mCard->attributes();
    removePositionalContAttrBuffsBySource(source);
    sendChangedAttrs(oldAttrs);

    if (mCard->zone()->name() == "stage" && mCard->power() <= 0)
        mCard->player()->triggerRuleAction(RuleAction::InsufficientPower, mCard);

    removeAbilityAsPositionalContBuffBySource(source);
    removePositionalContBoolAttrChangeBySource(source);

    for (auto it = cardBuffs.begin(); it != cardBuffs.end();) {
        if ((*it)->positional && (*it)->source == source)
            it = removeBuff(it);
        else
            ++it;
    }
}

void CardBuffManager::removePositionalContAttrBuffsBySource(ServerCard *source) {
    for (auto it = mContBuffs.begin(); it != mContBuffs.end();) {
        if (it->positional && it->source == source) {
            mCard->changeAttr(it->attr, -it->value);
            it = mContBuffs.erase(it);
        } else {
            ++it;
        }
    }
}

void CardBuffManager::removeAbilityAsPositionalContBuffBySource(ServerCard *source) {
    for (auto it = mAbilityBuffs.begin(); it != mAbilityBuffs.end();) {
        if (it->positional && it->source == source && it->duration == 0) {
            removeAbility(it->abilityId);
            it = mAbilityBuffs.erase(it);
        } else {
            ++it;
        }
    }
}

void CardBuffManager::removePositionalContBoolAttrChangeBySource(ServerCard *source) {
    for (auto it = mBoolAttrChanges.begin(); it != mBoolAttrChanges.end();) {
        if (it->positional && it->source == source && it->duration == 0) {
            auto type = it->type;
            it = mBoolAttrChanges.erase(it);
            if (!hasBoolAttrChange(type)) {
                mCard->changeBoolAttribute(type, false);
                sendBoolAttrChange(type, false);
            }
        } else {
            ++it;
        }
    }
}

void CardBuffManager::endOfTurnEffectValidation() {
    validateAttrBuffs();
    validateAbilities();
    validateBoolAttrChanges();

    for (auto it = cardBuffs.begin(); it != cardBuffs.end();) {
        if (shouldSkipEffectValidation(**it)) {
            ++it;
            continue;
        }

        if (!(*it)->duration || --(*it)->duration) {
            ++it;
            continue;
        }

        it = removeBuff(it);
    }
}

void CardBuffManager::validateCannotStand() {
    std::vector<BoolAttributeType> changedParams;
    auto it = mBoolAttrChanges.begin();
    while (it != mBoolAttrChanges.end()) {
        if (it->type != BoolAttributeType::CannotStand) {
            ++it;
            continue;
        }

        if (!it->duration) {
            ++it;
            continue;
        }

        auto type = it->type;
        it = mBoolAttrChanges.erase(it);
        if (!hasBoolAttrChange(type)) {
            mCard->changeBoolAttribute(type, false);
            sendBoolAttrChange(type, false);
        }
    }
}

CardBuffManager::CardBuffs::iterator
CardBuffManager::removeBuff(CardBuffs::iterator it) {
    auto removedBuff = std::move(*it);
    it = cardBuffs.erase(it);
    removedBuff->undo(this, mCard);
    return it;
}

bool CardBuffManager::shouldSkipEffectValidation(const Buff &buff) {
    return false;
}

void CardBuffManager::validateAttrBuffs() {
    auto oldAttrs = mCard->attributes();
    for (auto &buff: mBuffs) {
        if (--buff.duration == 0)
            mCard->changeAttr(buff.attr, -buff.value);
    }
    std::erase_if(mBuffs, [](const AttributeChange &o){ return o.duration <= 0; });
    sendChangedAttrs(oldAttrs);

    if (mCard->zone()->name() == "stage" && mCard->power() <= 0)
        mCard->player()->triggerRuleAction(RuleAction::InsufficientPower, mCard);
}

void CardBuffManager::validateAbilities() {
    auto it = mAbilityBuffs.begin();
    while (it != mAbilityBuffs.end()) {
        if (!it->duration || --it->duration) {
            ++it;
            continue;
        }

        removeAbility(it->abilityId);
        it = mAbilityBuffs.erase(it);
    }

    for (auto &a: mCard->abilities())
        a.activationTimes = 0;
}

void CardBuffManager::validateBoolAttrChanges() {
    std::vector<BoolAttributeType> changedParams;
    auto it = mBoolAttrChanges.begin();
    while (it != mBoolAttrChanges.end()) {
        if (it->type == BoolAttributeType::CannotStand) {
            ++it;
            continue;
        }

        if (!it->duration || --it->duration) {
            ++it;
            continue;
        }

        auto type = it->type;
        it = mBoolAttrChanges.erase(it);
        if (!hasBoolAttrChange(type)) {
            mCard->changeBoolAttribute(type, false);
            sendBoolAttrChange(type, false);
        }
    }
}

bool CardBuffManager::hasBoolAttrChange(BoolAttributeType type) const {
    auto sameType = std::find_if(mBoolAttrChanges.begin(), mBoolAttrChanges.end(),
                                 [type](const BoolAttributeChange &el) { return el.type == type; });
    return sameType != mBoolAttrChanges.end();
}

bool CardBuffManager::hasBoolAttrChangeEx(BoolAttributeType type) const {
    return std::any_of(cardBuffs.begin(), cardBuffs.end(),
                                    [type](const auto &el) {
        if (el->buffType == BuffType::BoolAttrChange) {
            auto boolAttrBuff = dynamic_cast<BoolAttributeChangeEx*>(el.get());
            if (boolAttrBuff && boolAttrBuff->attrType == type)
                return true;
        }
        return false;
    });
}

TraitChange::TraitChange(asn::TraitModificationType type, std::string trait, int duration)
    : Buff(BuffType::TraitChange, duration), type(type), trait(trait) {
    traitChangeId = generateId();
}

void CardBuffManager::addTraitChange(const TraitChange *traitChange) {
    const auto it = std::find_if(appliedTraitChanges.begin(), appliedTraitChanges.end(),
                 [traitChange](const TraitChange *elem) {
        return traitChange->traitChangeId == elem->traitChangeId;
    });
    if (it == appliedTraitChanges.end())
        return;
    appliedTraitChanges.push_back(traitChange);
}

void CardBuffManager::removeTraitChange(const TraitChange *traitChange) {
    std::erase_if(appliedTraitChanges, [traitChange](const TraitChange *elem) {
        return traitChange->traitChangeId == elem->traitChangeId;
    });
}
