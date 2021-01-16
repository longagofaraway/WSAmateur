#include "print.h"

std::string printAutoAbility(const AutoAbility a) {
    std::string res = "[A] ";

    res += printTrigger(a.trigger);

    for (const auto &effect : a.effects) {
        res += printEffect(effect);
    }

    return res;
}

std::string printAbility(const Ability &a) {
    switch (a.type) {
    case AbilityType::Auto:
        return printAutoAbility(std::get<AutoAbility>(a.ability));
    }

    return "";
}
