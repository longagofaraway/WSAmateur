#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "condition.h"
#include "cost.h"
#include "effect.h"
#include "trigger.h"

namespace asn {

enum class AbilityItem : uint8_t {
    EndTag = 0,
    Cost,
    Trigger,
    Effect,
    Keyword,
    Activation
};

enum class Keyword : uint8_t {
    Encore = 1,
    Cxcombo,
    Brainstorm,
    Backup,
    Experience,
    Resonance,
    Bond,
    Replay,
    Alarm,
    Change,
    Assist,
    Memory
};

struct AutoAbility {
    int activationTimes;
    std::vector<Keyword> keywords;
    std::optional<Cost> cost;
    std::vector<Trigger> triggers;
    std::vector<Effect> effects;
};

struct ContAbility {
    std::vector<Keyword> keywords;
    std::vector<Effect> effects;
};

struct ActAbility {
    std::vector<Keyword> keywords;
    Cost cost;
    std::vector<Effect> effects;
};

struct EventAbility {
    std::vector<Keyword> keywords;
    std::vector<Effect> effects;
};


struct Ability {
    AbilityType type;
    std::variant<std::monostate,
                 AutoAbility,
                 ContAbility,
                 ActAbility,
                 EventAbility
    > ability;
};

}
