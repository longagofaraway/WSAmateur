#include "effectInit.h"

#include "language_parser.h"

namespace {
const asn::Effect kEmptyEffect = asn::Effect{.type=asn::EffectType::AbilityGain};
asn::Target upTo1Char() {
    asn::Card card;
    card.cardSpecifiers.push_back(asn::CardSpecifier{.type=asn::CardSpecifierType::CardType,
                                                     .specifier=asn::CardType::Char});
    asn::Target t{.type = asn::TargetType::SpecificCards};
    t.targetSpecification = asn::TargetSpecificCards{.mode=asn::TargetMode::Any,
            .number=asn::Number{.mod=asn::NumModifier::UpTo,.value=1},
            .cards=card};
    return t;
}
}

decltype(asn::Effect::effect) getDefaultEffect(asn::EffectType type) {
    asn::Target thisCardTarget;
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
        tp.target = upTo1Char();
        tp.placeType = asn::PlaceType::SpecificPlace;
        tp.place = asn::Place{.pos=asn::Position::NotSpecified,
                .zone=asn::Zone::WaitingRoom,
                .owner=asn::Player::Player};
        ef.targets.push_back(tp);
        return ef;
    }
    case asn::EffectType::MoveCard:{
        auto ef = asn::MoveCard();
        ef.executor = asn::Player::Player;
        ef.from = asn::Place{.pos=asn::Position::NotSpecified,.zone=asn::Zone::WaitingRoom,.owner=asn::Player::Player};
        ef.to.push_back(asn::Place{.pos=asn::Position::NotSpecified,.zone=asn::Zone::Hand,.owner=asn::Player::Player});
        ef.target = upTo1Char();
        ef.order = asn::Order::NotSpecified;
        return ef;
    }
    case asn::EffectType::Look:{
        auto ef = asn::Look{};
        ef.number = asn::Number{.mod=asn::NumModifier::UpTo,.value=3};
        ef.place = asn::Place{.pos=asn::Position::Top,.zone=asn::Zone::Deck,.owner=asn::Player::Player};
        ef.valueType = asn::ValueType::RawValue;
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

decltype(asn::Effect::effect) nullifyOptionalFields(asn::EffectType type, decltype(asn::Effect::effect) effect) {
    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto &ef = std::get<asn::AttributeGain>(effect);
        if (ef.gainType != asn::ValueType::Multiplier) {
            ef.modifier = std::nullopt;
        }
        break;
    }
    case asn::EffectType::Look: {
        auto &ef = std::get<asn::Look>(effect);
        if (ef.valueType != asn::ValueType::Multiplier) {
            ef.multiplier = std::nullopt;
        }
        break;
    }
    case asn::EffectType::RevealCard: {
        auto &ef = std::get<asn::RevealCard>(effect);
        if (ef.type != asn::RevealType::FromHand) {
            ef.card = std::nullopt;
        }
        break;
    }
    case asn::EffectType::DealDamage: {
        auto &ef = std::get<asn::DealDamage>(effect);
        if (ef.damageType != asn::ValueType::Multiplier) {
            ef.modifier = std::nullopt;
        }
        break;
    }
    }
    return effect;
}
