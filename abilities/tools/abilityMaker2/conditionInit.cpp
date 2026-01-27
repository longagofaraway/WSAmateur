#include "conditionInit.h"

#include <stdexcept>

#include "language_parser.h"


decltype(asn::Condition::cond) getDefaultCondition(asn::ConditionType type) {
    asn::Place stage{.pos=asn::Position::NotSpecified,.zone=asn::Zone::Stage,.owner=asn::Player::Player};
    asn::Number number{.mod = asn::NumModifier::ExactMatch, .value = 1};
    asn::Number number3OrMore{.mod = asn::NumModifier::AtLeast, .value = 3};
    asn::Number number2OrMore{.mod = asn::NumModifier::AtLeast, .value = 2};
    asn::Card level3OrHigher;
    asn::CardSpecifier sp{.type = asn::CardSpecifierType::Level, .specifier = asn::Level{.value = number3OrMore}};
    level3OrHigher.cardSpecifiers.push_back(sp);
    asn::Card character;
    character.cardSpecifiers.push_back(asn::CardSpecifier{.type=asn::CardSpecifierType::CardType,.specifier=asn::CardType::Char});
    asn::Target thisCardTarget;
    thisCardTarget.type = asn::TargetType::ThisCard;
    asn::Target battleOpponent;
    battleOpponent.type = asn::TargetType::BattleOpponent;
    battleOpponent.targetSpecification = asn::TargetSpecificCards{.mode=asn::TargetMode::Any, .number=number};

    switch (type) {
    case asn::ConditionType::IsCard:{
        auto condition = asn::ConditionIsCard();
        condition.target = battleOpponent;
        condition.neededCard.push_back(level3OrHigher);
        return condition;
    }
    case asn::ConditionType::HaveCards:{
        auto condition = asn::ConditionHaveCard();
        condition.invert = false;
        condition.who = asn::Player::Player;
        condition.howMany = number2OrMore;
        condition.whichCards = character;
        condition.where = stage;
        condition.excludingThis = true;
        return condition;
    }
    case asn::ConditionType::And:{
        return asn::ConditionAnd();
    }
    case asn::ConditionType::Or:{
        return asn::ConditionOr();
    }
    case asn::ConditionType::NoCondition:
    case asn::ConditionType::InBattleWithThis:
    case asn::ConditionType::DuringCardsFirstTurn:
    case asn::ConditionType::PerformedInFull:
        return std::monostate();
    case asn::ConditionType::SumOfLevels:
        return asn::ConditionSumOfLevels{.equalOrMoreThan = 2};
    case asn::ConditionType::CardsLocation:{
        auto condition = asn::ConditionCardsLocation{};
        condition.target = thisCardTarget;
        condition.place = {.pos=asn::Position::BackRow,.zone=asn::Zone::Stage,.owner=asn::Player::Player};
        return condition;
    }
    case asn::ConditionType::DuringTurn:
        return asn::ConditionDuringTurn{.player=asn::Player::Player};
    case asn::ConditionType::CheckMilledCards: {
        auto condition = asn::ConditionCheckMilledCards{};
        condition.number = asn::Number{.mod = asn::NumModifier::ExactMatch, .value = 1};
        condition.card = character;
        return condition;
    }
    case asn::ConditionType::RevealedCard: {
        auto condition = asn::ConditionRevealCard{};
        condition.number = asn::Number{.mod = asn::NumModifier::AtLeast, .value = 1};
        condition.card = asn::Card{};
        return condition;
    }
    case asn::ConditionType::PlayersLevel: {
        auto condition = asn::ConditionPlayersLevel{};
        condition.value = number3OrMore;
        return condition;
    }
    case asn::ConditionType::CardMoved: {
        auto condition = asn::ConditionCardMoved{};
        condition.player = asn::Player::Player;
        condition.from = asn::Zone::Deck;
        condition.to = asn::Zone::Hand;
        return condition;
    }
    case asn::ConditionType::HasMarkers: {
        auto condition = asn::ConditionHasMarkers{};
        condition.target = thisCardTarget;
        condition.number = asn::Number{.mod = asn::NumModifier::AtLeast, .value = 1};
        return condition;
    }
    }

    throw std::runtime_error("unhandled ConditionType");
}

asn::Condition getConditionFromPreset(QString preset) {
    asn::Condition condition;
    condition.type = parse(preset.toStdString(), formats::To<asn::ConditionType>{});
    condition.cond = getDefaultCondition(condition.type);
    return condition;
}
