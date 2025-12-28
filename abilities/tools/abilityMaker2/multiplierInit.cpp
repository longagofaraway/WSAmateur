#include "multiplierInit.h"

#include "language_parser.h"


decltype(asn::Multiplier::specifier) getDefaultMultiplier(asn::MultiplierType type) {
    auto others = asn::Target();
    others.type = asn::TargetType::SpecificCards;
    asn::TargetSpecificCards spec;
    spec.mode = asn::TargetMode::AllOther;
    spec.number = asn::Number{.mod=asn::NumModifier::ExactMatch,.value=1};
    asn::CardSpecifier character{.type=asn::CardSpecifierType::CardType, .specifier=asn::CardType::Char};
    asn::CardSpecifier ownerPlayer{.type=asn::CardSpecifierType::Owner, .specifier=asn::Player::Player};
    spec.cards.cardSpecifiers.push_back(character);
    spec.cards.cardSpecifiers.push_back(ownerPlayer);
    others.targetSpecification = spec;

    auto inFrontOf = asn::Target();
    others.type = asn::TargetType::SpecificCards;
    asn::TargetSpecificCards inFrontSpec;
    inFrontSpec.mode = asn::TargetMode::InFrontOfThis;
    inFrontSpec.number = asn::Number{.mod=asn::NumModifier::ExactMatch,.value=1};
    inFrontSpec.cards.cardSpecifiers.push_back(character);
    inFrontSpec.cards.cardSpecifiers.push_back(ownerPlayer);
    inFrontOf.targetSpecification = inFrontSpec;

    auto lastMoved = asn::Target{.type=asn::TargetType::LastMovedCards};

    decltype(asn::Multiplier::specifier) multiplier;
    switch (type) {
    case asn::MultiplierType::ForEach: {
        auto m = asn::ForEachMultiplier();
        m.target = std::make_shared<asn::Target>(others);
        m.placeType = asn::PlaceType::SpecificPlace;
        m.place = asn::Place{.pos=asn::Position::NotSpecified,.zone=asn::Zone::Stage,.owner=asn::Player::Player};
        multiplier = m;
        break;
    }
    case asn::MultiplierType::TimesLevel: {
        multiplier = std::monostate();
        break;
    }
    case asn::MultiplierType::AddLevel: {
        auto m = asn::AddLevelMultiplier();
        m.target = std::make_shared<asn::Target>(inFrontOf);
        multiplier = m;
        break;
    }
    case asn::MultiplierType::AddTriggerNumber: {
        auto m = asn::AddTriggerNumberMultiplier();
        m.target = std::make_shared<asn::Target>(lastMoved);
        multiplier = m;
        break;
    }
    case asn::MultiplierType::PreviousDamage: {
        multiplier = std::monostate();
        break;
    }
    }
    return multiplier;
}
