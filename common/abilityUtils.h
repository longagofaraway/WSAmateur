#pragma once

#include <string_view>

#include <QString>

#include "ability.pb.h"
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
CardState stateToProtoState(asn::State state);

bool checkNumber(const asn::Number &numObj, int n);
bool checkCard(const std::vector<asn::CardSpecifier> &specs, const CardBase &card);
bool checkTargetMode(asn::TargetMode mode, const ServerCard *thisCard, const ServerCard *card);
