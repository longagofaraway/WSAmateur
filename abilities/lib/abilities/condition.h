#pragma once

#include <variant>
#include <vector>

#include "basicTypes.h"
#include "asnCard.h"
#include "target.h"

struct Condition;

struct ConditionAnd {
    std::vector<Condition> cond;
};

struct ConditionOr {
    std::vector<Condition> cond;
};

struct ConditionIsCard{
    Target target;
    std::vector<AsnCard> neededCard; // array here instead of ORing conditions
};

struct ConditionHaveCard {
    bool invert;
    AsnPlayer who;
    Number howMany;
    AsnCard whichCards;
    AsnZone where;
    bool excludingThis;
};

struct ConditionSumOfLevels {
    int moreThan;
};

struct ConditionCardsLocation {
    Target target;
    Place place;
}; 

struct ConditionDuringTurn {
    Owner player;
};

struct ConditionCheckOpenedCards {
    Number number;
    AsnCard card;
};

struct ConditionRevealCard {
     Number number;
     AsnCard card;
};

struct ConditionPlayersLevel {
    Number value;
};

enum class ConditionType : uint8_t {
    NoCondition = 0,
    IsCard,
    HaveCards,
    And,
    Or,
    InBattleWithThis,
    SumOfLevels,
    CardsLocation,
    DuringTurn,
    CheckOpenedCards,
    RevealedCard,
    PlayersLevel
};

struct Condition {
    ConditionType type;
    std::variant<
        ConditionIsCard,
        ConditionHaveCard,
        ConditionSumOfLevels,
        ConditionCardsLocation,
        ConditionDuringTurn,
        ConditionCheckOpenedCards,
        ConditionRevealCard,
        ConditionPlayersLevel,
        ConditionAnd,
        ConditionOr
    > cond;
};
