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
    case Zone::Clock: return "clock";
    case Zone::Deck: return "deck";
    case Zone::Hand: return "hand";
    case Zone::Level: return "level zone";
    case Zone::Memory: return "memory";
    case Zone::Stage: return "stage";
    case Zone::Stock: return "stock";
    case Zone::WaitingRoom: return "waiting room";
    case Zone::Climax: return "climax zone";
    default: throw PrintingException();
    }
}
