#pragma once

#include <string_view>

#include <QString>

#include "ability.pb.h"
#include "attackType.pb.h"
#include "cardAttribute.pb.h"

#include "abilities.h"
#include "attributeChange.h"
#include "serverCard.h"
#include "playerBuffManager.h"

class ProtoAbility;

std::string_view asnZoneToString(asn::Zone zone);
QString asnZoneToReadableString(asn::Zone zone);
QString placeToReadableString(const asn::Place &place);
uint32_t abilityHash(const ProtoAbility &a);
ProtoCardAttribute attrTypeToProto(asn::AttributeType t);
asn::Player protoPlayerToPlayer(ProtoOwner player);
asn::State protoStateToState(CardState state);
asn::AttackType protoAttackTypeToAttackType(AttackType attackType);
CardState stateToProtoState(asn::State state);
asn::Player reversePlayer(asn::Player p);
ProtoCardBoolAttribute getProtoBoolAttrType(BoolAttributeType type);
ProtoPlayerAttribute getProtoPlayerAttrType(PlayerAttrType type);

bool checkNumber(const asn::Number &numObj, int n);
bool checkCard(const std::vector<asn::CardSpecifier> &specs, const CardBase &card);
bool checkTargetMode(asn::TargetMode mode, const ServerCard *thisCard, const ServerCard *card);
bool isPositional(const asn::Target &t);
bool checkPlace(const ServerCard *card, const asn::Place &place);
