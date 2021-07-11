#pragma once

//#include <vector>

#include "abilities.h"

struct ActivatedAbility;
class CardZone;

asn::Ability decodeAbility(const std::string &buf);
void highlightAllCards(CardZone *zone, bool highlight);
void selectAllCards(CardZone *zone, bool select);
int highlightEligibleCards(CardZone *zone, const std::vector<asn::CardSpecifier> &specs,
                           asn::TargetMode mode, const ActivatedAbility &a);

