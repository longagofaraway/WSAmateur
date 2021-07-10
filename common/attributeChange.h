#pragma once

#include "abilities.h"

class ServerCard;

class AttributeChange {
public:
    asn::AttributeType attr;
    int value;
    int duration;

    AttributeChange(asn::AttributeType attr, int delta, int duration)
        : attr(attr), value(delta), duration(duration) {}
};

class ContAttributeChange {
public:
    // card, that gives this buff
    ServerCard *source;
    // ability of source that gives the buff
    int abilityId;
    asn::AttributeType attr;
    int value;
    bool positional;

    ContAttributeChange(ServerCard *card, int abilityId, asn::AttributeType attr, int value, bool positional = false)
        : source(card), abilityId(abilityId), attr(attr), value(value), positional(positional) {}
};

inline bool operator==(const ContAttributeChange &lhs, const ContAttributeChange &rhs) {
    return lhs.source == rhs.source && lhs.abilityId == rhs.abilityId &&
        lhs.attr == rhs.attr;
}

class AbilityAsContBuff {
public:
    // card, that gives this buff
    ServerCard *source;
    // ability of source that gives the buff
    int sourceAbilityId;
    // ability given by source
    int abilityId;
    bool positional;

    AbilityAsContBuff(ServerCard *card, int abilityId, bool positional)
        : source(card), sourceAbilityId(abilityId), positional(positional) {}
};

inline bool operator==(const AbilityAsContBuff &lhs, const AbilityAsContBuff &rhs) {
    return lhs.source == rhs.source && lhs.sourceAbilityId == rhs.sourceAbilityId;
}
