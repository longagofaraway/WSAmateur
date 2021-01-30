#include "print.h"

#include <exception>

class PrintingException : public std::exception {};

std::string printDigit(int8_t value) {
    switch (value) {
    case 1: return "one";
    case 2: return "two";
    case 3: return "three";
    case 4: return "four";
    case 5: return "five";
    case 6: return "six";
    default: throw PrintingException();
    }
}

std::string printTrait(const std::string &trait) {
    return '<' + trait + '>';
}

std::string printZone(AsnZone zone) {
    switch (zone) {
    case AsnZone::Clock: return "clock";
    case AsnZone::Deck: return "deck";
    case AsnZone::Hand: return "hand";
    case AsnZone::Level: return "level zone";
    case AsnZone::Memory: return "memory";
    case AsnZone::Stage: return "stage";
    case AsnZone::Stock: return "stock";
    case AsnZone::WaitingRoom: return "waiting room";
    case AsnZone::Climax: return "climax zone";
    default: throw PrintingException();
    }
}

std::string printEffects(const std::vector<Effect> &effects) {
    std::string s;

    for (int i = 0; i < (int)effects.size(); ++i) {
        if (i != 0 && effects[i].cond.type != ConditionType::NoCondition) {
            s[s.size() - 1] = '.';
            s.push_back(' ');
        } else if (i != 0) {
            if (i < (int)effects.size() - 1) {
                s[s.size() - 1] = ',';
                s.push_back(' ');
            } else if (i == (int)effects.size() - 1) {
                s[s.size() - 1] = ',';
                s += " and ";
            }
        }
        s += printEffect(effects[i]);
    }

    return s;
}
