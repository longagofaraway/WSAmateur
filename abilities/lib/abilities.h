#pragma once

#include "abilities/ability.h"

namespace asn {
std::string printAbility(const Ability &a);
std::vector<uint8_t> encodeAbility(const Ability &a);
Ability decodeAbility(const std::vector<uint8_t> &buf);
}
