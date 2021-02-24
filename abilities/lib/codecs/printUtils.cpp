#include "print.h"

#include <exception>

using namespace asn;

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

std::string printZone(Zone zone) {
    switch (zone) {
    case Zone::NotSpecified: return "";
    case Zone::Clock: return "clock";
    case Zone::Deck: return "deck";
    case Zone::Hand: return "hand";
    case Zone::Level: return "level zone";
    case Zone::Memory: return "memory";
    case Zone::Stage: return "stage";
    case Zone::Stock: return "stock";
    case Zone::WaitingRoom: return "waiting room";
    case Zone::Climax: return "climax area";
    default: throw PrintingException();
    }
}

std::string printTarget(const Target &t) {
    std::string s;

    if (t.type == TargetType::ThisCard)
        s += "this card ";
    else if (t.type == TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        s += printCard(spec.cards, false) + " ";
    }

    return s;
}

std::string printNumber(const Number &n) {
    std::string s;
    if (n.mod == NumModifier::UpTo)
        s += "up to ";
    else if (n.mod == NumModifier::AtLeast)
        s += "at least ";
    s += std::to_string(n.value) + " ";
    return s;
}

std::string printState(State s) {
    switch (s) {
    case State::Reversed:
        return "<img align=\"middle\" src=\"qrc:///resources/images/reverse.png\">";
    case State::Standing:
        return "<img align=\"middle\" src=\"qrc:///resources/images/stand.png\">";
    case State::Rested:
        return "<img align=\"middle\" src=\"qrc:///resources/images/rest.png\">";
    }
    return "";
}

std::string printPlayer(const Player &p) {
    std::string s;

    switch (p) {
    case Player::Player:
        s += "your ";
        break;
    case Player::Both:
        s += "your or your opponent's ";
        break;
    case Player::Opponent:
        s += "your opponent's ";
        break;
    case Player::NotSpecified:
    default:
        break;
    }

    return s;
}

