#include "attributeChange.h"

#include "abilityEvents.pb.h"

#include "abilityUtils.h"
#include "serverCard.h"
#include "serverPlayer.h"


void Buff::fillBaseFields(Buff *newBuff, const Buff *sourceBuff) const {
    newBuff->source = sourceBuff->source;
    newBuff->abilityId = sourceBuff->abilityId;
    newBuff->positional = sourceBuff->positional;
    newBuff->duration = sourceBuff->duration;
    newBuff->buffType = sourceBuff->buffType;
}

void TriggerIconBuff::apply(ServerCard *card) const {
    card->addTriggerIcon(icon);
}

void TriggerIconBuff::undo(CardBuffManager *buffManager, ServerCard *card) const {
    card->removeTriggerIcon(icon);
}

std::unique_ptr<Buff> TriggerIconBuff::clone() const {
    auto buff = std::make_unique<TriggerIconBuff>();
    fillBaseFields(buff.get(), this);
    buff->icon = icon;
    return buff;
}

namespace {
void sendBoolAttrChange(ServerCard *card, BoolAttributeType type, bool value) {
    EventSetCardBoolAttr event;
    event.set_card_pos(card->pos());
    event.set_zone(card->zone()->name());
    event.set_attr(getProtoBoolAttrType(type));
    event.set_value(value);
    card->player()->sendToBoth(event);
}
}

void BoolAttributeChangeEx::apply(ServerCard *card) const {
    if (card->boolAttrByType(attrType))
        return;

    card->changeBoolAttribute(attrType, true);
    sendBoolAttrChange(card, attrType, true);
}

void BoolAttributeChangeEx::undo(CardBuffManager *buffManager, ServerCard *card) const {
    if (!buffManager->hasBoolAttrChangeEx(attrType)) {
        card->changeBoolAttribute(attrType, false);
        sendBoolAttrChange(card, attrType, false);
    }
}

std::unique_ptr<Buff> BoolAttributeChangeEx::clone() const {
    auto buff = std::make_unique<BoolAttributeChangeEx>(attrType, 0);
    fillBaseFields(buff.get(), this);
    buff->attrType = attrType;
    return buff;
}

void TraitChange::apply(ServerCard *card) const {
    card->buffManager()->addTraitChange(this);
    // sendTraits
}

void TraitChange::undo(CardBuffManager *buffManager, ServerCard *card) const {
    card->buffManager()->removeTraitChange(this);
    // sendTraits
}

std::unique_ptr<Buff> TraitChange::clone() const {
    auto buff = std::make_unique<TraitChange>(type, trait, 0);
    fillBaseFields(buff.get(), this);
    buff->traitChangeId = traitChangeId;
    return buff;
}
