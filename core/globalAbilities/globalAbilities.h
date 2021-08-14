#pragma once

#include "abilities.h"

enum class RuleAction {
    RefreshPoint,
    InsufficientPower
};

enum class GlobalAbility {
    Encore
};

void decodeGlobalAbilities();
asn::Ability triggerAbility(asn::TriggerIcon trigger);
asn::Ability ruleActionAbility(RuleAction ruleAction);
asn::Ability globalAbility(GlobalAbility ability);
