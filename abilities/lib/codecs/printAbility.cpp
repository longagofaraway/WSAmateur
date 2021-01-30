#include "print.h"

std::string printAutoAbility(const AutoAbility a) {
    std::string s = "[A] ";

    s += printTrigger(a.trigger);

    s += printEffects(a.effects);

    s[4] = std::toupper(s[4]);
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

namespace asn {
std::string printAbility(const Ability &a) {
    switch (a.type) {
    case AbilityType::Auto:
        return printAutoAbility(std::get<AutoAbility>(a.ability));
    case AbilityType::Event:
        return printEventAbility(std::get<EventAbility>(a.ability));
    }

    return "";
}
}
