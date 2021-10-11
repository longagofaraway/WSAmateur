#pragma once

#include <memory>
#include <optional>

namespace asn {

enum class NumModifier : uint8_t {
    NotSpecified = 0,
    ExactMatch,
    UpTo,
    AtLeast
};

struct Number {
    NumModifier mod;
    int8_t value;
};

inline bool operator==(const Number &lhs, const Number &rhs) {
    if (lhs.mod != rhs.mod || lhs.value != rhs.value)
        return false;
    return true;
}
inline bool operator!=(const Number &lhs, const Number &rhs) { return !(lhs == rhs); }

}
