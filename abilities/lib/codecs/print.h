#pragma once

#include "abilities.h"

std::string printDigit(int8_t value);
std::string printTrait(const std::string &trait);
std::string printCard(const asn::Card &c, bool plural);
std::string printCondition(const asn::Condition &c);
std::string printTrigger(const asn::Trigger &t);
std::string printEffect(const asn::Effect &e);
std::string printZone(asn::Zone zone);
std::string printEffects(const std::vector<asn::Effect> &effects);

std::string printMoveCard(const asn::MoveCard &e);
