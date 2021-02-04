#pragma once

#include <string_view>

#include <QString>

#include "abilities.h"

class ProtoAbility;

std::string_view asnZoneToString(asn::Zone zone);
QString asnZoneToReadableString(asn::Zone zone);
uint32_t abilityHash(const ProtoAbility &a);
