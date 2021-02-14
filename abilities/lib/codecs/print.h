#pragma once

#include "abilities.h"

// utils
std::string printDigit(int8_t value);
std::string printTrait(const std::string &trait);
std::string printCard(const asn::Card &c, bool plural, bool article = true);
std::string printNumber(const asn::Number &n);
std::string printZone(asn::Zone zone);

std::string printCondition(const asn::Condition &c);
std::string printCost(const asn::Cost &c);
std::string printEffect(const asn::Effect &e);
std::string printEffects(const std::vector<asn::Effect> &effects);
std::string printTrigger(const asn::Trigger &t);

// effects
std::string printMoveCard(const asn::MoveCard &e);
std::string printDrawCard(const asn::DrawCard &e);
