#include "print.h"

using namespace asn;


template<typename T>
std::string printSpecificAbility(const T &a) {
    std::string s;

    if constexpr (std::is_same_v<T, AutoAbility>)
        s += "【AUTO】 ";
    else if constexpr (std::is_same_v<T, ContAbility>)
        s += "【CONT】 ";
    else if constexpr (std::is_same_v<T, ActAbility>)
        s += "【ACT】 ";

    s += printKeywords(a.keywords);
    size_t prefixLen = s.size();

    if constexpr (std::is_same_v<T, AutoAbility>) {
        if (a.cost) {
            s += printCost(*a.cost);
            prefixLen = s.size();
        }

        s += printTrigger(a.trigger);
    }

    bool isBackup = false;
    if constexpr (std::is_same_v<T, ActAbility>) {
        if (!(a.keywords.size() && a.keywords[0] == Keyword::Backup)) {
            s += printCost(a.cost);
            prefixLen = s.size();
        } else {
            isBackup = true;
        }
    }

    s += printEffects(a.effects);

    if constexpr (std::is_same_v<T, ActAbility>) {
        if (isBackup)
            s += printCost(a.cost);
    }

    if (!isBackup)
        s[prefixLen] = std::toupper(s[prefixLen]);
    if (s[s.size() - 1] == ' ')
        s.pop_back();
    if (s[s.size() - 1] == ',')
        s.pop_back();
    s.push_back('.');
    return s;
}

std::string printAbility(const Ability &a) {
    switch (a.type) {
    case AbilityType::Auto:
        return printSpecificAbility(std::get<AutoAbility>(a.ability));
    case AbilityType::Cont:
        return printSpecificAbility(std::get<ContAbility>(a.ability));
    case AbilityType::Event:
        return printSpecificAbility(std::get<EventAbility>(a.ability));
    case AbilityType::Act:
        return printSpecificAbility(std::get<ActAbility>(a.ability));
    }

    return "";
}
