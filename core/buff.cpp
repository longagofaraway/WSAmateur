#include "attributeChange.h"

#include "serverCard.h"


void Buff::fillBaseFields(Buff *newBuff, const Buff *sourceBuff) const {
    newBuff->source = sourceBuff->source;
    newBuff->abilityId = sourceBuff->abilityId;
    newBuff->positional = sourceBuff->positional;
    newBuff->duration = sourceBuff->duration;
    newBuff->type = sourceBuff->type;
}

void TriggerIconBuff::apply(ServerCard *card) const {
    card->addTriggerIcon(icon);
}

void TriggerIconBuff::undo(ServerCard *card) const {
    card->removeTriggerIcon(icon);
}

std::unique_ptr<Buff> TriggerIconBuff::clone() const {
    auto buff = std::make_unique<TriggerIconBuff>();
    fillBaseFields(buff.get(), this);
    buff->icon = icon;
    return buff;
}
