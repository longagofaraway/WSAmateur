#include <unordered_map>
#include <vector>

#include "abilities.h"
#include "../cardInfo.h"

std::unordered_map<Trigger, asn::Ability> gTriggerAbilities;

static std::unordered_map<Trigger, std::vector<uint8_t>> triggerAbilities = {
    { Trigger::Door, {
          #include "door"
      }
    }
};

void decodeGlobalAbilities() {
    gTriggerAbilities.clear();
    for (const auto &pair: triggerAbilities)
        gTriggerAbilities.emplace(pair.first, decodeAbility(pair.second));
}
