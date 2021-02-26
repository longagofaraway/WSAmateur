#include "abilities.h"
#include "../cardInfo.h"

enum class RuleAction {
    RefreshPoint
};

void decodeGlobalAbilities();
asn::Ability triggerAbility(asn::TriggerIcon trigger);
asn::Ability ruleActionAbility(RuleAction ruleAction);
