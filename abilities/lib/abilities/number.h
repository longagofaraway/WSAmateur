#pragma once

#include <memory>
#include <optional>

#include "multiplier.h"

namespace asn {

enum class NumModifier : uint8_t {
    NotSpecified = 0,
    ExactMatch,
    UpTo,
    AtLeast,
    Multiplier
};

struct Number {
    NumModifier mod;
    int8_t value;
    std::optional<Multiplier> multiplier;
};

inline bool operator==(const Number &lhs, const Number &rhs) {
    if (lhs.mod != rhs.mod || lhs.value != rhs.value)
        return false;
    return true;
}
inline bool operator!=(const Number &lhs, const Number &rhs) { return !(lhs == rhs); }

}
