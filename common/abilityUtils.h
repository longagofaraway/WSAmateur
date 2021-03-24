#pragma once

#include <string_view>

#include <QString>

#include "abilities.pb.h"
#include "cardAttribute.pb.h"

#include "abilities.h"
#include "serverCard.h"

class ProtoAbility;

std::string_view asnZoneToString(asn::Zone zone);
QString asnZoneToReadableString(asn::Zone zone);
uint32_t abilityHash(const ProtoAbility &a);
ProtoCardAttribute attrTypeToProto(asn::AttributeType t);
asn::Player protoPlayerToPlayer(ProtoOwner player);
asn::State protoStateToState(CardState state);

bool checkNumber(const asn::Number &numObj, int n);
bool checkCard(const std::vector<asn::CardSpecifier> &specs, const CardBase &card);
