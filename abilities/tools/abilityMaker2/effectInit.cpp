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
