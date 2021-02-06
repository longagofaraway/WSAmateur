#include "globalAbilities.h"

#include <unordered_map>
#include <vector>

#include <abilities.h>


namespace {
std::unordered_map<asn::TriggerIcon, asn::Ability> gTriggerAbilities;
std::unordered_map<asn::TriggerIcon, std::vector<uint8_t>> gTriggerBinAbilities = {
    { asn::TriggerIcon::Door, {
          #include "door"
      }
    },
    { asn::TriggerIcon::Choice, {
          #include "choice"
      }
    },
    { asn::TriggerIcon::Wind, {
          #include "wind"
      }
    },
    { asn::TriggerIcon::Treasure, {
          #include "treasure"
      }
    }
};
bool gAbilitiesDecoded = false;
}

void decodeGlobalAbilities() {
    gTriggerAbilities.clear();
    for (const auto &pair: gTriggerBinAbilities)
        gTriggerAbilities.emplace(pair.first, decodeAbility(pair.second));
    gAbilitiesDecoded = true;
}

asn::Ability globalAbility(asn::TriggerIcon trigger) {
    if (!gAbilitiesDecoded)
        decodeGlobalAbilities();

    return gTriggerAbilities.at(trigger);
}
