#pragma once

#include <abilities.h>

#include "abilityContext.h"
#include "cardImprint.h"

struct DelayedAbility {
    asn::AutoAbility ability;
    CardImprint thisCard;
    std::string uniqueId;
    AbilityContext context;
    int duration;
    bool isShotTrigger;
};
