#include "serverPlayer.h"

#include "gameEvent.pb.h"

#include "abilityUtils.h"

void ServerPlayer::endOfTurnEffectValidation() {
    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;
        auto oldAttrs = card->attributes();
        card->validateBuffs();
        sendChangedAttrs(card, oldAttrs);
        auto &abs = card->abilities();
        auto it = abs.rbegin();
        while (it != abs.rend()) {
            if (it->permanent)
                break;
            if (--it->duration != 0) {
                ++it;
                continue;
            }

            EventRemoveAbility event;
            event.set_cardid(card->pos());
            event.set_zone(card->zone()->name());
            event.set_abilityid(std::distance(abs.begin(), (it+1).base()));
            sendToBoth(event);

            it = std::reverse_iterator(abs.erase((++it).base()));
        }
    }
}

void ServerPlayer::sendAttrChange(ServerCard *card, asn::AttributeType attr) {
    EventSetCardAttr event;
    event.set_stageid(card->pos());
    event.set_attr(attrTypeToProto(attr));
    event.set_value(card->attrByType(attr));
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


void ServerPlayer::addAttributeBuff(asn::AttributeType attr, int pos, int delta, int duration) {
    auto stage = zone("stage");
    auto card = stage->card(pos);
    if (!card)
        return;

    card->addAttrBuff(attr, delta, duration);

    sendAttrChange(card, attr);
}

void ServerPlayer::addContAttributeBuff(ServerCard *card, ServerCard *source, int abilityId, asn::AttributeType attr, int delta) {
    if (!card->addContAttrBuff(source, abilityId, attr, delta))
        return;

    sendAttrChange(card, attr);
}

void ServerPlayer::removeContAttributeBuff(ServerCard *card, ServerCard *source, int abilityId, asn::AttributeType attr) {
    card->removeContAttrBuff(source, abilityId, attr);
    sendAttrChange(card, attr);
}

void ServerPlayer::changeAttribute(ServerCard *card, asn::AttributeType attr, int delta) {
    card->changeAttr(attr, delta);
    sendAttrChange(card, attr);
}
