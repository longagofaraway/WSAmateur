#include "print.h"

using namespace asn;

PrintState gPrintState;

std::string printAbility(const Ability &a, CardType cardType) {
    switch (a.type) {
    case AbilityType::Auto:
        return printSpecificAbility(std::get<AutoAbility>(a.ability), cardType);
    case AbilityType::Cont:
        return printSpecificAbility(std::get<ContAbility>(a.ability), cardType);
    case AbilityType::Event:
        return printSpecificAbility(std::get<EventAbility>(a.ability), cardType);
    case AbilityType::Act:
        return printSpecificAbility(std::get<ActAbility>(a.ability), cardType);
    }

    return "";
}

std::string printActivationTimes(int activationTimes) {
    std::string s = "This ability activates up to ";

    s += std::to_string(activationTimes) + " time";
    if (activationTimes > 1)
        s += "s";
    s += " per turn. ";

    return s;
}
