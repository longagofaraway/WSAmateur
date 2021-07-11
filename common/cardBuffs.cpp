#include "serverPlayer.h"

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
        card->validateBuffs();
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

                removeAbilityFromCard(card, it->id);

                it = std::reverse_iterator(abs.erase((++it).base()));
            } else {
                ++it;
            }
        }
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

void ServerPlayer::addContAttributeBuff(ServerCard *card, ServerCard *source, int abilityId, asn::AttributeType attr, int delta, bool positional) {
    if (!card->addContAttrBuff(source, abilityId, attr, delta, positional))
        return;

    card->player()->sendAttrChange(card, attr);
}

void ServerPlayer::removeContAttributeBuff(ServerCard *card, ServerCard *source, int abilityId, asn::AttributeType attr) {
    card->removeContAttrBuff(source, abilityId, attr);
    card->player()->sendAttrChange(card, attr);
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
    }
}

void ServerPlayer::removePositionalContBuffsFromCard(ServerCard *card) {
    card->removePositionalContAttrBuffs();
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
