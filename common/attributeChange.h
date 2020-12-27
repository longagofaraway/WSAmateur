#pragma once

#include "cardAttribute.pb.h"

class AttributeChange {
    CardAttribute mAttr;
    int mValue;
    int mDuration;

public:
    AttributeChange(CardAttribute attr, int delta, int duration)
        : mAttr(attr), mValue(delta), mDuration(duration) {}
};
