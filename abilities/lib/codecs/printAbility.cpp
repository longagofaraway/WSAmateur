#include "print.h"

using namespace asn;

std::string printAutoAbility(const AutoAbility a) {
    std::string prefix = "【AUTO】 ";
    std::string s = prefix;

    if (a.cost)
        s += printCost(*a.cost);

    s += printTrigger(a.trigger);

    s += printEffects(a.effects);

    s[prefix.size()] = std::toupper(s[prefix.size()]);
    if (s[s.size() - 1] == ' ')
        s.pop_back();
    if (s[s.size() - 1] == ',')
        s.pop_back();
    s.push_back('.');
    return s;
}

std::string printEventAbility(const EventAbility a) {
    std::string s = "";

    s += printEffects(a.effects);

    if (s.size()) {
        s[0] = std::toupper(s[0]);
        if (s[s.size() - 1] == ' ')
            s.pop_back();
        if (s[s.size() - 1] == ',')
            s.pop_back();
        s.push_back('.');
    }
    return s;
}

std::string printContAbility(const ContAbility a) {
    std::string prefix = "【CONT】 ";
    std::string s = prefix;

    s += printEffects(a.effects);

    if (s.size()) {
        s[prefix.size()] = std::toupper(s[prefix.size()]);
        if (s[s.size() - 1] == ' ')
            s.pop_back();
        if (s[s.size() - 1] == ',')
            s.pop_back();
        s.push_back('.');
    }
    return s;
}

std::string printAbility(const Ability &a) {
    switch (a.type) {
    case AbilityType::Auto:
        return printAutoAbility(std::get<AutoAbility>(a.ability));
    case AbilityType::Cont:
        return printContAbility(std::get<ContAbility>(a.ability));
    case AbilityType::Event:
        return printEventAbility(std::get<EventAbility>(a.ability));
    }

    return "";
}
