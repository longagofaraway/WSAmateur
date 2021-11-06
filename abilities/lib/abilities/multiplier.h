#pragma once

#include <optional>

#include "basicTypes.h"

namespace asn {

struct Target;

enum class MultiplierType : uint8_t {
    ForEach = 1,
    TimesLevel,
    MarkersPutInWrThisWay
};

struct ForEachMultiplier {
    // shared_ptr to break circular dependecy
    std::shared_ptr<Target> target;
    PlaceType placeType;
    std::optional<Place> place;
};

struct Multiplier {
    MultiplierType type;
    std::optional<ForEachMultiplier> specifier;
};

}
