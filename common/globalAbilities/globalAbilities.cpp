#include "globalAbilities.h"

#include <unordered_map>
#include <vector>

#include "../cardInfo.h"


namespace {
std::unordered_map<Trigger, asn::Ability> gTriggerAbilities;
std::unordered_map<Trigger, std::vector<uint8_t>> gTriggerBinAbilities = {
    { Trigger::Door, {
          #include "door"
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

asn::Ability globalAbility(Trigger trigger) {
    if (!gAbilitiesDecoded)
        decodeGlobalAbilities();

    return gTriggerAbilities.at(trigger);
}
