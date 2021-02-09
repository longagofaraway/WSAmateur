#pragma once

#include <string_view>

#include <QString>

#include "cardAttribute.pb.h"

#include "abilities.h"
#include "serverCard.h"

class ProtoAbility;

std::string_view asnZoneToString(asn::Zone zone);
QString asnZoneToReadableString(asn::Zone zone);
uint32_t abilityHash(const ProtoAbility &a);
ProtoCardAttribute attrTypeToProto(asn::AttributeType t);

bool checkCard(const std::vector<asn::CardSpecifier> &specs, const CardBase &card);
