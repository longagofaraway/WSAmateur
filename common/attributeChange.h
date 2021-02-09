#pragma once

#include "abilities.h"

class AttributeChange {
public:
    asn::AttributeType mAttr;
    int mValue;
    int mDuration;

    AttributeChange(asn::AttributeType attr, int delta, int duration)
        : mAttr(attr), mValue(delta), mDuration(duration) {}
};
