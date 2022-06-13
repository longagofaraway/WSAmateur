#include "encode.h"

#include "encDecUtils.h"

using namespace asn;

void encodeCostItem(const CostItem &c, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(c.type));
    if (c.type == CostType::Stock)
        buf.push_back(zzenc_8(std::get<StockCost>(c.costItem).value));
    else
        encodeEffect(std::get<Effect>(c.costItem), buf);
}

void encodeCost(const Cost &c, Buf &buf) {
    encodeArray(c.items, buf, encodeCostItem);
}

void encodeKeyword(const Keyword &k, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(k));
}

void encodeAutoAbility(const AutoAbility &a, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(AbilityItem::Activation));
    buf.push_back(static_cast<uint8_t>(a.activationTimes));

    if (a.keywords.size()) {
        buf.push_back(static_cast<uint8_t>(AbilityItem::Keyword));
        encodeArray(a.keywords, buf, encodeKeyword);
    }

    if (a.cost) {
        buf.push_back(static_cast<uint8_t>(AbilityItem::Cost));
        encodeCost(*a.cost, buf);
    }

    buf.push_back(static_cast<uint8_t>(AbilityItem::Trigger));
    encodeArray(a.triggers, buf, encodeTrigger);

    buf.push_back(static_cast<uint8_t>(AbilityItem::Effect));
    encodeArray(a.effects, buf, encodeEffect);

    buf.push_back(static_cast<uint8_t>(AbilityItem::EndTag));
}

void encodeContAbility(const ContAbility &a, Buf &buf) {
    if (a.keywords.size()) {
        buf.push_back(static_cast<uint8_t>(AbilityItem::Keyword));
        encodeArray(a.keywords, buf, encodeKeyword);
    }

    buf.push_back(static_cast<uint8_t>(AbilityItem::Effect));
    encodeArray(a.effects, buf, encodeEffect);

    buf.push_back(static_cast<uint8_t>(AbilityItem::EndTag));
}

void encodeActAbility(const ActAbility &a, Buf &buf) {
    if (a.keywords.size()) {
        buf.push_back(static_cast<uint8_t>(AbilityItem::Keyword));
        encodeArray(a.keywords, buf, encodeKeyword);
    }

    buf.push_back(static_cast<uint8_t>(AbilityItem::Cost));
    encodeCost(a.cost, buf);

    buf.push_back(static_cast<uint8_t>(AbilityItem::Effect));
    encodeArray(a.effects, buf, encodeEffect);

    buf.push_back(static_cast<uint8_t>(AbilityItem::EndTag));
}

void encodeEventAbility(const EventAbility &a, Buf &buf) {
    if (a.keywords.size()) {
        buf.push_back(static_cast<uint8_t>(AbilityItem::Keyword));
        encodeArray(a.keywords, buf, encodeKeyword);
    }

    buf.push_back(static_cast<uint8_t>(AbilityItem::Effect));
    encodeArray(a.effects, buf, encodeEffect);

    buf.push_back(static_cast<uint8_t>(AbilityItem::EndTag));
}

Buf encodeAbility(const Ability &a) {
    std::vector<uint8_t> buf;

    encodeAbility(a, buf);

    return buf;
}

void encodeAbility(const Ability &a, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(a.type));

    switch (a.type) {
    case AbilityType::Auto:
        encodeAutoAbility(std::get<AutoAbility>(a.ability), buf);
        break;
    case AbilityType::Cont:
        encodeContAbility(std::get<ContAbility>(a.ability), buf);
        break;
    case AbilityType::Act:
        encodeActAbility(std::get<ActAbility>(a.ability), buf);
        break;
    case AbilityType::Event:
        encodeEventAbility(std::get<EventAbility>(a.ability), buf);
        break;
    }
}
