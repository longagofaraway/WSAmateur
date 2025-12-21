#include "effectInit.h"

#include "language_parser.h"

namespace {
const asn::Effect kEmptyEffect = asn::Effect{.type=asn::EffectType::AbilityGain};
}

decltype(asn::Effect::effect) getDefaultEffect(asn::EffectType type) {
    auto thisCardTarget = asn::Target();
    thisCardTarget.type = asn::TargetType::ThisCard;
    switch (type) {
    case asn::EffectType::AttributeGain:{
        auto ef = asn::AttributeGain();
        ef.target = thisCardTarget;
        ef.type = asn::AttributeType::Power;
        ef.gainType = asn::ValueType::RawValue;
        ef.value = 1000;
        ef.duration = 1;
        return ef;
    }
    case asn::EffectType::ChooseCard:{
        auto ef = asn::ChooseCard();
        ef.executor = asn::Player::Player;
        asn::TargetAndPlace tp;
        asn::Target t;
        asn::Card card;
        card.cardSpecifiers.push_back(asn::CardSpecifier{.type=asn::CardSpecifierType::CardType,
                                                         .specifier=asn::CardType::Char});
        t.type = asn::TargetType::SpecificCards;
        t.targetSpecification = asn::TargetSpecificCards{.mode=asn::TargetMode::Any,
                .number=asn::Number{.mod=asn::NumModifier::UpTo,.value=1},
                .cards=card};
        tp.target = t;
        tp.placeType = asn::PlaceType::SpecificPlace;
        tp.place = asn::Place{.pos=asn::Position::NotSpecified,
                .zone=asn::Zone::WaitingRoom,
                .owner=asn::Player::Player};
        ef.targets.push_back(tp);
        return ef;
    }
    case asn::EffectType::PayCost:
        return asn::PayCost();
    }

    return kEmptyEffect.effect;
}

asn::Effect getEffectFromPreset(QString preset) {
    asn::Effect effect;
    effect.type = parse(preset.toStdString(), formats::To<asn::EffectType>{});
    effect.effect = getDefaultEffect(effect.type);
    return effect;
}
