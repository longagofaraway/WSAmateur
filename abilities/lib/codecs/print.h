#pragma once

#include "abilities.h"

using namespace asn;

std::string printDigit(int8_t value);
std::string printTrait(const std::string &trait);
std::string printCard(const Card &c, bool plural);
std::string printCondition(const Condition &c);
std::string printTrigger(const Trigger &t);
std::string printEffect(const Effect &e);
std::string printZone(Zone zone);
std::string printEffects(const std::vector<Effect> &effects);
