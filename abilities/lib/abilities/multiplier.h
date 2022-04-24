#pragma once

#include <optional>

#include "basicTypes.h"

namespace asn {

struct Target;

enum class MultiplierType : uint8_t {
    ForEach = 1,
    TimesLevel,
    AddLevel,
    AddTriggerNumber,
    PreviousDamage
};

struct ForEachMultiplier {
    // shared_ptr to break circular dependecy
    std::shared_ptr<Target> target;
    PlaceType placeType;
    std::optional<Place> place;
};

struct AddLevelMultiplier {
    std::shared_ptr<Target> target;
};

struct AddTriggerNumberMultiplier {
    std::shared_ptr<Target> target;
    TriggerIcon triggerIcon;
};

struct Multiplier {
    MultiplierType type;
    std::variant<std::monostate,
                 ForEachMultiplier,
                 AddLevelMultiplier,
                 AddTriggerNumberMultiplier
    > specifier;
};

}
