#pragma once

#include <optional>

#include "asnCard.h"
#include "basicTypes.h"
#include "number.h"

namespace asn {

enum class TargetType : uint8_t {
    ThisCard = 1,
    ChosenCards,
    SpecificCards,
    RestOfTheCards,
    BattleOpponent,
    MentionedCards,
    CharInBattle,
    OppositeThis
};

enum class TargetMode : uint8_t {
    Any = 0,
    All,
    AllOther,
    InFrontOfThis,
    FrontRow,
    BackRow
};

struct TargetSpecificCards {
    TargetMode mode;
    Number number;
    Card cards;
};

struct Target {
    TargetType type;
    std::optional<TargetSpecificCards> targetSpecification;
};

}
