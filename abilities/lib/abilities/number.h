#pragma once

#include <memory>
#include <optional>

namespace asn {

struct Target;

enum class MultiplierType : uint8_t {
    ForEach = 1,
    TimesLevel,
    MarkersPutInWrThisWay
};

struct Multiplier {
    MultiplierType type;
    // shared_ptr to break circular dependecy
    std::shared_ptr<Target> forEach;
    Zone zone;
};

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
