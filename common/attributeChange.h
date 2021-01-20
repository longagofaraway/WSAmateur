#pragma once

#include "cardAttribute.pb.h"

class AttributeChange {
public:
    CardAttribute mAttr;
    int mValue;
    int mDuration;

    AttributeChange(CardAttribute attr, int delta, int duration)
        : mAttr(attr), mValue(delta), mDuration(duration) {}
};
