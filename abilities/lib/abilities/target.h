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
    OppositeThis,
    LastMovedCards,
    MentionedInTrigger
};

enum class TargetMode : uint8_t {
    Any = 0,
    All,
    AllOther,
    InFrontOfThis,
    FrontRow,
    BackRow,
    FrontRowOther,
    BackRowOther
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

inline bool operator==(const TargetSpecificCards &lhs, const TargetSpecificCards &rhs) {
    if (lhs.mode != rhs.mode || lhs.number != rhs.number)
        return false;
    return true;
}
inline bool operator!=(const TargetSpecificCards &lhs, const TargetSpecificCards &rhs) { return !(lhs == rhs); }

inline bool operator==(const Target &lhs, const Target &rhs) {
    if (lhs.type != rhs.type)
        return false;
    if (lhs.type == TargetType::SpecificCards) {
        if (*lhs.targetSpecification != *rhs.targetSpecification)
            return false;
    }
    return true;
}
inline bool operator!=(const Target &lhs, const Target &rhs) { return !(lhs == rhs); }

}
