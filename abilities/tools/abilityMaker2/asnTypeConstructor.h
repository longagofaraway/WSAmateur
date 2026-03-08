#pragma once

#include "abilities.h"

class AsnTypeConstructor {
public:
    asn::Number getNumber();
    asn::Target getTarget();
    asn::TargetAndPlace getTargetAndPlace();
    asn::Card getCard();
    asn::Place getPlace();
    asn::Condition getCondition();
    asn::Effect getEffect();
    asn::Ability getAbility();
    asn::SearchTarget getSearchTarget();
    asn::EventAbility getEventAbility();
};
