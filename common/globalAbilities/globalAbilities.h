#include "abilities.h"
#include "../cardInfo.h"

enum class RuleAction {
    RefreshPoint
};

enum class GlobalAbility {
    Encore
};

void decodeGlobalAbilities();
asn::Ability triggerAbility(asn::TriggerIcon trigger);
asn::Ability ruleActionAbility(RuleAction ruleAction);
asn::Ability globalAbility(GlobalAbility ability);
