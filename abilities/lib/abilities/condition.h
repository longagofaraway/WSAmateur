#pragma once

#include <variant>
#include <vector>

#include "basicTypes.h"
#include "asnCard.h"
#include "target.h"

namespace asn {

struct Condition;

struct ConditionAnd {
    std::vector<Condition> cond;
};

struct ConditionOr {
    std::vector<Condition> cond;
};

struct ConditionIsCard{
    Target target;
    std::vector<Card> neededCard; // array here instead of ORing conditions
};

struct ConditionHaveCard {
    bool invert;
    Player who;
    Number howMany;
    Card whichCards;
    Place where;
    bool excludingThis;
};

struct ConditionSumOfLevels {
    int equalOrMoreThan;
};

struct ConditionCardsLocation {
    Target target;
    Place place;
}; 

struct ConditionDuringTurn {
    Player player;
};

struct ConditionCheckMilledCards {
    Number number;
    Card card;
};

struct ConditionRevealCard {
    Number number;
    Card card;
};

struct ConditionPlayersLevel {
    Number value;
};

struct ConditionCardMoved {
    Player player;
    Zone from;
    Zone to;
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
    CheckMilledCards,
    RevealedCard,
    PlayersLevel,
    DuringCardsFirstTurn,
    CardMoved,
    PerformedInFull
};

struct Condition {
    ConditionType type;
    std::variant<
        std::monostate,
        ConditionIsCard,
        ConditionHaveCard,
        ConditionSumOfLevels,
        ConditionCardsLocation,
        ConditionDuringTurn,
        ConditionCheckMilledCards,
        ConditionRevealCard,
        ConditionPlayersLevel,
        ConditionAnd,
        ConditionOr,
        ConditionCardMoved
    > cond;
};

}
