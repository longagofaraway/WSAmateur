#pragma once

#include <string_view>

#include "abilities.h"

class ProtoAbility;

std::string_view asnZoneToString(asn::Zone zone);
uint32_t abilityHash(const ProtoAbility &a);
