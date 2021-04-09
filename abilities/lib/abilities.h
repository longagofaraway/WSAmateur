#pragma once

#include "abilities/ability.h"

std::string printAbility(const asn::Ability &a, asn::CardType cardType = asn::CardType::Char);
std::vector<uint8_t> encodeAbility(const asn::Ability &a);
asn::Ability decodeAbility(const std::vector<uint8_t> &buf);
