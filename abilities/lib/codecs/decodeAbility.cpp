#include "decode.h"

using namespace asn;

CostItem decodeCostItem(Iterator &it, Iterator end) {
    CostItem c;
    c.type = decodeEnum<CostType>(it, end);
    switch (c.type) {
    case CostType::Stock:
        c.costItem = StockCost{ decodeInt8(it, end) };
        break;
    case CostType::Effects:
        c.costItem = decodeEffect(it, end);
        break;
    default:
        throw DecodeException("unknown CostType");
    }

    return c;
}

Cost decodeCost(Iterator &it, Iterator end) {
    Cost c;
    c.items = decodeArray<CostItem>(it, end, decodeCostItem);
    return c;
}

AutoAbility decodeAutoAbility(Iterator &it, Iterator end) {
    AutoAbility a;

    bool abilityEnd = false;
    while (it != end && !abilityEnd) {
        auto t = decodeEnum<AbilityItem>(it, end);
        switch (t) {
        case AbilityItem::Activation:
            a.activationTimes = decodeEnum<int>(it, end);
            break;
        case AbilityItem::Keyword:
            a.keywords = decodeArray<Keyword>(it, end, decodeEnum<Keyword>);
            break;
        case AbilityItem::Cost:
            a.cost = decodeCost(it, end);
            break;
        case AbilityItem::Trigger:
            a.triggers = decodeArray<Trigger>(it, end, decodeTrigger);
            break;
        case AbilityItem::Effect:
            a.effects = decodeArray<Effect>(it, end, decodeEffect);
            break;
        case AbilityItem::EndTag:
            abilityEnd = true;
            break;
        default:
            throw DecodeException("unknown AbilityItem");
        }
    }

    return a;
}

ActAbility decodeActAbility(Iterator &it, Iterator end) {
    ActAbility a;

    bool abilityEnd = false;
    while (it != end && !abilityEnd) {
        auto t = decodeEnum<AbilityItem>(it, end);
        switch (t) {
        case AbilityItem::Keyword:
            a.keywords = decodeArray<Keyword>(it, end, decodeEnum<Keyword>);
            break;
        case AbilityItem::Cost:
            a.cost = decodeCost(it, end);
            break;
        case AbilityItem::Effect:
            a.effects = decodeArray<Effect>(it, end, decodeEffect);
            break;
        case AbilityItem::EndTag:
            abilityEnd = true;
            break;
        default:
            throw DecodeException("unknown AbilityItem");
        }
    }

    return a;
}

ContAbility decodeContAbility(Iterator &it, Iterator end) {
    ContAbility a;

    bool abilityEnd = false;
    while (it != end && !abilityEnd) {
        auto t = decodeEnum<AbilityItem>(it, end);
        switch (t) {
        case AbilityItem::Keyword:
            a.keywords = decodeArray<Keyword>(it, end, decodeEnum<Keyword>);
            break;
        case AbilityItem::Effect:
            a.effects = decodeArray<Effect>(it, end, decodeEffect);
            break;
        case AbilityItem::EndTag:
            abilityEnd = true;
            break;
        default:
            throw DecodeException("unknown AbilityItem");
        }
    }

    return a;
}

EventAbility decodeEventAbility(Iterator &it, Iterator end) {
    EventAbility a;

    bool abilityEnd = false;
    while (it != end && !abilityEnd) {
        auto t = decodeEnum<AbilityItem>(it, end);
        switch (t) {
        case AbilityItem::Keyword:
            a.keywords = decodeArray<Keyword>(it, end, decodeEnum<Keyword>);
            break;
        case AbilityItem::Effect:
            a.effects = decodeArray<Effect>(it, end, decodeEffect);
            break;
        case AbilityItem::EndTag:
            abilityEnd = true;
            break;
        default:
            throw DecodeException("unknown AbilityItem");
        }
    }

    return a;
}

Ability decodeAbility(Iterator &it, Iterator end) {
    Ability a;
    a.type = decodeEnum<AbilityType>(it, end);
    switch (a.type) {
    case AbilityType::Auto:
        a.ability = decodeAutoAbility(it, end);
        break;
    case AbilityType::Act:
        a.ability = decodeActAbility(it, end);
        break;
    case AbilityType::Cont:
        a.ability = decodeContAbility(it, end);
        break;
    case AbilityType::Event:
        a.ability = decodeEventAbility(it, end);
        break;
    }
    return a;
}

Ability decodeAbility(const std::vector<uint8_t> &buf) {
    auto it = buf.begin();
    return decodeAbility(it, buf.end());
}
