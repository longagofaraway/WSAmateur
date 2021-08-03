#include "serverPlayer.h"

#include "abilityEvents.pb.h"
#include "gameEvent.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"

void ServerPlayer::endOfTurnEffectValidation() {
    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        card->setTriggerCheckTwice(false);

        auto oldAttrs = card->attributes();
        card->validateAttrBuffs();
        sendChangedAttrs(card, oldAttrs);
        auto &abs = card->abilities();
        auto it = abs.rbegin();
        while (it != abs.rend()) {
            it->activationTimes = 0;
            // remove temp abilities
            if (!it->permanent && it->duration > 0) {
                if (--it->duration != 0) {
                    ++it;
                    continue;
                }

                if (it->active && it->ability.type == asn::AbilityType::Cont) {
                    AbilityPlayer a(this);
                    a.setThisCard(CardImprint(card->zone()->name(), card));
                    a.setAbilityId(it->id);
                    a.revertContAbility(std::get<asn::ContAbility>(it->ability.ability));
                }

                EventRemoveAbility event;
                event.set_card_pos(card->pos());
                event.set_zone(card->zone()->name());
                event.set_ability_id(it->id);
                sendToBoth(event);

                it = std::reverse_iterator(abs.erase((++it).base()));
            } else {
                ++it;
            }
        }

        auto changedAttrs = card->validateBoolAttrChanges();
        for (auto changedAttr: changedAttrs)
            sendBoolAttrChange(card->pos(), changedAttr, false);
    }
}

void ServerPlayer::sendAttrChange(ServerCard *card, asn::AttributeType attr) {
    EventSetCardAttr event;
    event.set_card_pos(card->pos());
    event.set_zone(card->zone()->name());
    event.set_attr(attrTypeToProto(attr));
    event.set_value(card->attrByType(attr));
    if (card->zone()->type() == ZoneType::PrivateZone)
        sendGameEvent(event);
    else
        sendToBoth(event);
}

void ServerPlayer::sendChangedAttrs(ServerCard *card, std::tuple<int, int, int> oldAttrs) {
    auto [oldPower, oldSoul, oldLevel] = oldAttrs;
    if (card->power() != oldPower)
        sendAttrChange(card, asn::AttributeType::Power);
    if (card->soul() != oldSoul)
        sendAttrChange(card, asn::AttributeType::Soul);
    if (card->level() != oldLevel)
        sendAttrChange(card, asn::AttributeType::Level);
}

void ServerPlayer::addAttributeBuff(ServerCard *card, asn::AttributeType attr, int delta, int duration) {
    card->addAttrBuff(attr, delta, duration);

    card->player()->sendAttrChange(card, attr);
}

void ServerPlayer::addBoolAttrChange(ServerCard *card, BoolAttributeType type, int duration) {
    bool attr = card->boolAttrByType(type);
    card->addBoolAttrChange(type, duration);
    bool newAttr = card->boolAttrByType(type);
    if (newAttr == attr)
        return;
    card->player()->sendBoolAttrChange(card->pos(), type, newAttr);
}

void ServerPlayer::addContAttributeBuff(ServerCard *card, ServerCard *source, int abilityId, asn::AttributeType attr, int delta, bool positional) {
    if (!card->addContAttrBuff(source, abilityId, attr, delta, positional))
        return;

    card->player()->sendAttrChange(card, attr);
}

void ServerPlayer::addContBoolAttrChange(ServerCard *card, ServerCard *source, int abilityId, BoolAttributeType type, bool positional) {
    bool attr = card->boolAttrByType(type);
    if (!card->addContBoolAttrChange(source, abilityId, type, positional))
        return;
    bool newAttr = card->boolAttrByType(type);
    if (newAttr == attr)
        return;

    card->player()->sendBoolAttrChange(card->pos(), type, newAttr);
}

void ServerPlayer::removeContAttributeBuff(ServerCard *card, ServerCard *source, int abilityId, asn::AttributeType attr) {
    card->removeContAttrBuff(source, abilityId, attr);
    card->player()->sendAttrChange(card, attr);
}

void ServerPlayer::removeContBoolAttrChange(ServerCard *card, ServerCard *source, int abilityId, BoolAttributeType type) {
    bool attr = card->boolAttrByType(type);
    card->removeContBoolAttrChange(source, abilityId, type);
    bool newAttr = card->boolAttrByType(type);
    if (newAttr == attr)
        return;

    card->player()->sendBoolAttrChange(card->pos(), type, newAttr);
}

void ServerPlayer::removePositionalContBuffsBySource(ServerCard *source) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        auto oldAttrs = card->attributes();
        card->removePositionalContAttrBuffsBySource(source);
        while (true) {
            bool abilityRemoved = false;
            int abilityId = card->removeAbilityAsPositionalContBuffBySource(source, abilityRemoved);
            if (!abilityRemoved)
                break;
            card->player()->removeAbilityFromCard(card, abilityId);
        }
        sendChangedAttrs(card, oldAttrs);

        auto changedBoolAttrs = card->removePositionalContBoolAttrChangeBySource(source);
        for (auto changedAttr: changedBoolAttrs)
            sendBoolAttrChange(card->pos(), changedAttr, false);
    }
}

void ServerPlayer::removePositionalContBuffsFromCard(ServerCard *card) {
    card->removePositionalContAttrBuffs();

    auto changedBoolAttrs = card->removePositionalContBoolAttrChanges();
    for (auto changedAttr: changedBoolAttrs)
        sendBoolAttrChange(card->pos(), changedAttr, false);

    while (true) {
        bool abilityRemoved = false;
        int abilityId = card->removeAbilityAsPositionalContBuff(abilityRemoved);
        if (!abilityRemoved)
            break;
        card->player()->removeAbilityFromCard(card, abilityId);
    }
}

void ServerPlayer::addAbilityAsContBuff(ServerCard *card,
                                        ServerCard *source,
                                        int sourceAbilityId,
                                        const asn::Ability &ability,
                                        bool positional) {
    bool abilityCreated = false;
    auto &buff = card->addAbilityAsContBuff(source, sourceAbilityId, positional, abilityCreated);
    if (!abilityCreated)
        return;

    int newId = card->player()->addAbilityToCard(card, ability, 0);
    buff.abilityId = newId;
}

void ServerPlayer::removeAbilityAsContBuff(ServerCard *card, ServerCard *source, int sourceAbilityId) {
    bool abilityRemoved = false;
    int abilityId = card->removeAbilityAsContBuff(source, sourceAbilityId, abilityRemoved);
    if (abilityRemoved)
        card->player()->removeAbilityFromCard(card, abilityId);
}
