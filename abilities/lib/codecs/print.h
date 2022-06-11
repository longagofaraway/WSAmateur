#pragma once

#include "abilities.h"

struct PrintState {
    asn::Number mentionedCardsNumber;
    asn::Number chosenCardsNumber;
    int lastMovedCardsNumber = 0;
    bool battleOpponentMentioned = false;
    bool abilityChainingFirst = false;
    bool abilityChainingSecond = false;
    bool mandatory = true;
    asn::AbilityType type = asn::AbilityType::Cont;

    PrintState() = default;
    PrintState(const asn::Number &num1, const asn::Number &num2)
        : mentionedCardsNumber(num1), chosenCardsNumber(num2) {}
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
std::string printColor(asn::Color color);
std::string printForEachMultiplier(const asn::ForEachMultiplier &m, bool addOf = false);
std::string printAttackType(asn::AttackType t);
std::string printDuration(int duration);
std::string printPlace(asn::Place place);
std::string printFaceOrientation(asn::FaceOrientation orientation);
std::string printAbilityType(asn::AbilityType type);
bool haveExactName(const std::vector<asn::CardSpecifier> &s);

std::string printKeyword(asn::Keyword keyword);
std::string printKeywords(const std::vector<asn::Keyword> &keywords);
std::string printGlobalConditions(const std::vector<asn::Effect> &effects);
std::string printCondition(const asn::Condition &c, bool skipGlobalConditions = false);
std::string printCost(const asn::Cost &c);
std::string printEffect(const asn::Effect &e);
std::string printEffects(const std::vector<asn::Effect> &effects);
std::string printTrigger(const asn::Trigger &t);
std::string printTarget(const asn::Target &t, bool plural = false, bool nominative = false,
                        std::optional<bool> optArticle = {});
std::string printTargetAndPlace(const asn::Target &t, const asn::Place &p);

// effects
std::string printMoveCard(const asn::MoveCard &e);
std::string printDrawCard(const asn::DrawCard &e);
std::string printActivationTimes(int activationTimes);
std::string printChangeState(const asn::ChangeState &e);
std::string printRemoveMarker(const asn::RemoveMarker &e);

std::string printAutoAbilitySimplified(const asn::AutoAbility &ability);

template<typename T>
std::string printSpecificAbility(const T &a, asn::CardType cardType) {
    std::string s;

    static asn::Number defaultNumber;
    defaultNumber.mod = asn::NumModifier::ExactMatch;
    defaultNumber.value = 0;

    gPrintState = PrintState(defaultNumber, defaultNumber);

    if constexpr (std::is_same_v<T, asn::AutoAbility>) {
        s += "【AUTO】 ";
        gPrintState.type = asn::AbilityType::Auto;
    } else if constexpr (std::is_same_v<T, asn::ContAbility>) {
        if (cardType != asn::CardType::Event)
            s += "【CONT】 ";
        gPrintState.type = asn::AbilityType::Cont;
    } else if constexpr (std::is_same_v<T, asn::ActAbility>) {
        s += "【ACT】 ";
        gPrintState.type = asn::AbilityType::Act;
    } else {
        gPrintState.type = asn::AbilityType::Event;
    }

    s += printKeywords(a.keywords);
    size_t prefixLen = s.size();

    if constexpr (std::is_same_v<T, asn::AutoAbility>) {
        if (a.cost)
            s += printCost(*a.cost);
        if (a.activationTimes > 0)
            s += printActivationTimes(a.activationTimes);
        prefixLen = s.size();
        s += printGlobalConditions(a.effects);

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

    gPrintState = PrintState(defaultNumber, defaultNumber);

    return s;
}
