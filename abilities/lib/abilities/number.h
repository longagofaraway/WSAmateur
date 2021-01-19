#pragma once

#include <memory>
#include <optional>

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
    AsnZone zone;
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
