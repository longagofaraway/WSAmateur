#pragma once

#include <abilities.h>

#include "cardImprint.h"

struct DelayedAbility {
    asn::AutoAbility ability;
    CardImprint thisCard;
    std::string uniqueId;
    int duration;
};
