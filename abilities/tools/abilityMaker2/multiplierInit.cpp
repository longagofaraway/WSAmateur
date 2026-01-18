#include "multiplierInit.h"

#include "language_parser.h"


decltype(asn::Multiplier::specifier) getDefaultMultiplier(asn::MultiplierType type) {
    asn::Target thisCardTarget;
    thisCardTarget.type = asn::TargetType::ThisCard;

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

    auto lastMoved = asn::Target{.type=asn::TargetType::LastMovedCards};
    auto mentioned = asn::Target{.type=asn::TargetType::MentionedCards};

    decltype(asn::Multiplier::specifier) multiplier;
    switch (type) {
    case asn::MultiplierType::ForEach: {
        auto m = asn::ForEachMultiplier();
        m.target = std::make_shared<asn::Target>(others);
        m.placeType = asn::PlaceType::SpecificPlace;
        m.place = asn::Place{.pos=asn::Position::NotSpecified,.zone=asn::Zone::Stage,.owner=asn::Player::Player};
        // init all optional fields, so that there won't be uninitialized fields
        m.markerBearer = std::make_shared<asn::Target>(thisCardTarget);
        multiplier = m;
        break;
    }
    case asn::MultiplierType::TimesLevel: {
        multiplier = std::monostate();
        break;
    }
    case asn::MultiplierType::AddLevel: {
        auto m = asn::AddLevelMultiplier();
        m.target = std::make_shared<asn::Target>(mentioned);
        multiplier = m;
        break;
    }
    case asn::MultiplierType::AddTriggerNumber: {
        auto m = asn::AddTriggerNumberMultiplier();
        m.target = std::make_shared<asn::Target>(lastMoved);
        m.triggerIcon = asn::TriggerIcon::Soul;
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
