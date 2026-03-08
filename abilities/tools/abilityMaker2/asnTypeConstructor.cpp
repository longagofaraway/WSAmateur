#include "asnTypeConstructor.h"

asn::Number AsnTypeConstructor::getNumber() {
    return asn::Number{.mod=asn::NumModifier::ExactMatch,.value=1};
}
asn::Target AsnTypeConstructor::getTarget() {
    return asn::Target{.type=asn::TargetType::ThisCard};
}
asn::TargetAndPlace AsnTypeConstructor::getTargetAndPlace() {
    asn::TargetAndPlace tp;
    tp.place = getPlace();
    tp.placeType = asn::PlaceType::SpecificPlace;
    tp.target = getTarget();
    return tp;
}
asn::Card AsnTypeConstructor::getCard() {
    asn::Card card;
    card.cardSpecifiers.push_back(asn::CardSpecifier{.type=asn::CardSpecifierType::CardType,.specifier=asn::CardType::Char});
    return card;
}
asn::Place AsnTypeConstructor::getPlace() {
    return asn::Place{.pos=asn::Position::NotSpecified,.zone=asn::Zone::Stage,.owner=asn::Player::Player};
}
asn::SearchTarget AsnTypeConstructor::getSearchTarget() {
    asn::SearchTarget t;
    t.number = getNumber();
    t.cards.push_back(getCard());
    return t;
}

asn::Condition AsnTypeConstructor::getCondition() {
    return asn::Condition{.type=asn::ConditionType::NoCondition};
}
asn::Effect AsnTypeConstructor::getEffect() {
    return asn::Effect{.type=asn::EffectType::NotSpecified,.cond=getCondition()};
}
asn::Ability AsnTypeConstructor::getAbility() {
    return asn::Ability{.type=asn::AbilityType::Auto};
}
asn::EventAbility AsnTypeConstructor::getEventAbility() {
    return asn::EventAbility{};
}
