#pragma once

#include "abilities.h"

std::string printDigit(int8_t value);
std::string printTrait(const std::string &trait);
std::string printCard(const AsnCard &c, bool plural);
std::string printCondition(const Condition &c);
std::string printTrigger(const Trigger &t);
std::string printEffect(const Effect &e);
