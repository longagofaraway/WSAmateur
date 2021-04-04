#pragma once

#include "abilities.h"

// utils
std::string printDigit(int8_t value);
std::string printTrait(const std::string &trait);
std::string printCard(const asn::Card &c, bool plural = false, bool article = true, asn::TargetMode mode = asn::TargetMode::Any);
std::string printNumber(const asn::Number &n);
std::string printZone(asn::Zone zone);
std::string printPlayer(asn::Player p, asn::Player executor = asn::Player::Player);
std::string printState(asn::State s);
std::string printPhase(asn::Phase p);
bool haveExactName(const std::vector<asn::CardSpecifier> &s);

std::string printKeywords(const std::vector<asn::Keyword> &keywords);
std::string printCondition(const asn::Condition &c);
std::string printCost(const asn::Cost &c);
std::string printEffect(const asn::Effect &e);
std::string printEffects(const std::vector<asn::Effect> &effects);
std::string printTrigger(const asn::Trigger &t);
std::string printTarget(const asn::Target &t);

// effects
std::string printMoveCard(const asn::MoveCard &e);
std::string printDrawCard(const asn::DrawCard &e);

