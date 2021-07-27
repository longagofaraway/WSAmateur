#pragma once

#include "abilities.h"

struct PrintState {
    asn::Number mentionedCardsNumber;
    asn::Number chosenCardsNumber;
    bool battleOpponentMentioned = false;
    bool attributeGainChaining = false;
};
extern PrintState gPrintState;

// utils
std::string printDigit(int8_t value);
std::string printTrait(const std::string &trait);
std::string printCard(const asn::Card &c, bool plural = false, bool article = true, asn::TargetMode mode = asn::TargetMode::Any);
std::string printNumber(const asn::Number &n, bool lowerHigher = false);
std::string printZone(asn::Zone zone);
std::string printPlayer(asn::Player p, asn::Player executor = asn::Player::Player);
std::string printState(asn::State s);
std::string printPhase(asn::Phase p);
std::string printTriggerIcon(asn::TriggerIcon icon);
std::string printForEachMultiplier(const asn::ForEachMultiplier &m);
bool haveExactName(const std::vector<asn::CardSpecifier> &s);

std::string printKeywords(const std::vector<asn::Keyword> &keywords);
std::string printCondition(const asn::Condition &c);
std::string printCost(const asn::Cost &c);
std::string printEffect(const asn::Effect &e);
std::string printEffects(const std::vector<asn::Effect> &effects);
std::string printTrigger(const asn::Trigger &t);
std::string printTarget(const asn::Target &t, bool plural = false);

// effects
std::string printMoveCard(const asn::MoveCard &e);
std::string printDrawCard(const asn::DrawCard &e);
std::string printActivationTimes(int activationTimes);
std::string printChangeState(const asn::ChangeState &e);

template<typename T>
std::string printSpecificAbility(const T &a, asn::CardType cardType) {
    std::string s;

    gPrintState = { asn::Number(), asn::Number(), false, false };

    if constexpr (std::is_same_v<T, asn::AutoAbility>)
        s += "【AUTO】 ";
    else if constexpr (std::is_same_v<T, asn::ContAbility>) {
        if (cardType != asn::CardType::Event)
            s += "【CONT】 ";
    } else if constexpr (std::is_same_v<T, asn::ActAbility>)
        s += "【ACT】 ";

    s += printKeywords(a.keywords);
    size_t prefixLen = s.size();

    if constexpr (std::is_same_v<T, asn::AutoAbility>) {
        if (a.cost)
            s += printCost(*a.cost);
        if (a.activationTimes > 0)
            s += printActivationTimes(a.activationTimes);
        prefixLen = s.size();

        s += printTrigger(a.trigger);
    }

    bool isBackup = false;
    if constexpr (std::is_same_v<T, asn::ActAbility>) {
        if (!(a.keywords.size() && a.keywords[0] == asn::Keyword::Backup)) {
            s += printCost(a.cost);
            prefixLen = s.size();
        } else {
            isBackup = true;
        }
    }

    s += printEffects(a.effects);

    if constexpr (std::is_same_v<T, asn::ActAbility>) {
        if (isBackup)
            s += printCost(a.cost);
    }

    if (!isBackup)
        s[prefixLen] = std::toupper(s[prefixLen]);
    if (s.size() && s[s.size() - 1] == ' ')
        s.pop_back();
    if (s.size() && s[s.size() - 1] == ',')
        s.pop_back();
    s.push_back('.');

    gPrintState = { asn::Number(), asn::Number(), false, false };

    return s;
}
