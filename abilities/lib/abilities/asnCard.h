#pragma once

#include <string>
#include <variant>
#include <vector>

#include "basicTypes.h"
#include "multiplier.h"
#include "number.h"

namespace asn {

enum class CardSpecifierType : uint8_t {
    CardType = 1,
    Owner,
    Trait,
    ExactName,
    NameContains,
    Level,
    LevelHigherThanOpp,
    Color,
    Cost,
    TriggerIcon,
    HasMarker,
    Power,
    StandbyTarget,
    LevelWithMultiplier,
    State,
    SumOfLevelsLessThanDiffNamedEventsInMemory
};

enum class CardType : uint8_t {
    Char = 1,
    Climax,
    Event,
    Marker
};

enum class Color : uint8_t {
    Yellow = 1,
    Green,
    Red,
    Blue,
    Purple
};

struct Trait {
    std::string value;
};
struct ExactName {
    std::string value;
};
struct NameContains {
    std::string value;
};
struct Level {
    Number value;
};
struct CostSpecifier {
    Number value;
};
struct Power {
    Number value;
};
struct LevelWithMultiplier {
    Number value;
    Multiplier multiplier;
};

struct CardSpecifier {
    CardSpecifierType type;
    std::variant<
        std::monostate,
        CardType,
        Player,
        Trait,
        ExactName,
        NameContains,
        Level,
        Color,
        CostSpecifier,
        TriggerIcon,
        Power,
        LevelWithMultiplier,
        State
    > specifier;
};

struct Card {
    std::vector<CardSpecifier> cardSpecifiers;
};

}
