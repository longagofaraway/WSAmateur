#pragma once

#include "abilities.h"

class ServerCard;

class AttributeChange {
public:
    asn::AttributeType mAttr;
    int mValue;
    int mDuration;

    AttributeChange(asn::AttributeType attr, int delta, int duration)
        : mAttr(attr), mValue(delta), mDuration(duration) {}
};

class ContAttributeChange {
public:
    ServerCard *mSource;
    int mAbilityId;
    asn::AttributeType mAttr;
    int mValue;

    ContAttributeChange(ServerCard *card, int abilityId, asn::AttributeType attr, int value)
        : mSource(card), mAbilityId(abilityId), mAttr(attr), mValue(value) {}
};

inline bool operator==(const ContAttributeChange &lhs, const ContAttributeChange &rhs) {
    return lhs.mSource == rhs.mSource && lhs.mAbilityId == rhs.mAbilityId &&
        lhs.mAttr == rhs.mAttr;
}
