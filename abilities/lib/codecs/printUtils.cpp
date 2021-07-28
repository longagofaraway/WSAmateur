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
    return "&lt;" + trait + "&gt;";
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

std::string printTriggerIcon(TriggerIcon icon) {
    switch (icon) {
    case TriggerIcon::Bag: return "bag";
    case TriggerIcon::Book: return "book";
    case TriggerIcon::Choice: return "choice";
    case TriggerIcon::Door: return "door";
    case TriggerIcon::Gate: return "gate";
    case TriggerIcon::Shot: return "shot";
    case TriggerIcon::Soul: return "soul";
    case TriggerIcon::Standby: return "standby";
    case TriggerIcon::Treasure: return "treasure";
    case TriggerIcon::Wind: return "wind";
    default: throw PrintingException();
    }
}

std::string printTarget(const Target &t, bool plural) {
    std::string s;

    if (t.type == TargetType::ThisCard)
        s += "this card ";
    else if (t.type == TargetType::ChosenCards) {
        if (gPrintState.chosenCardsNumber.value == 1)
            s += "it ";
        else
            s += "them ";
    } else if (t.type == TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        bool article = true;
        if (spec.mode == TargetMode::AllOther) {
            plural = true;
            article = false;
        }
        s += printCard(spec.cards, plural, article, spec.mode) + " ";
    }

    return s;
}

std::string printNumber(const Number &n, bool lowerHigher) {
    std::string s;

    if (lowerHigher) {
        s += std::to_string(n.value);
        if (n.mod == NumModifier::UpTo)
            s += " or lower ";
        else if (n.mod == NumModifier::AtLeast)
            s += " or higher ";
    } else {
        if (n.mod == NumModifier::UpTo)
            s += "up to ";
        else if (n.mod == NumModifier::AtLeast)
            s += "at least ";
        s += std::to_string(n.value) + " ";
    }
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

std::string printPlayer(Player p, Player executor) {
    std::string s;

    switch (p) {
    case Player::Player:
        s += "your ";
        break;
    case Player::Both:
        s += "your or your opponent's ";
        break;
    case Player::Opponent:
        if (executor == Player::Player)
            s += "your opponent's ";
        else
            s += "their ";
        break;
    case Player::NotSpecified:
    default:
        break;
    }

    return s;
}

std::string printKeyword(Keyword keyword) {
    switch (keyword) {
    case Keyword::Alarm:
        return "Alarm";
    case Keyword::Assist:
        return "Assist";
    case Keyword::Backup:
        return "Backup";
    case Keyword::Bond:
        return "Bond";
    case Keyword::Brainstorm:
        return "Brainstorm";
    case Keyword::Change:
        return "Change";
    case Keyword::Cxcombo:
        return "CXCombo";
    case Keyword::Encore:
        return "Encore";
    case Keyword::Experience:
        return "Experience";
    case Keyword::Replay:
        return "Replay";
    case Keyword::Resonance:
        return "Resonance";
    default:
        return "";
    }
}

std::string printKeywords(const std::vector<Keyword> &keywords) {
    std::string s;

    for (auto keyword: keywords)
        s += "<b>" + printKeyword(keyword) + "</b> ";

    return s;
}

std::string printPhase(Phase p) {
    switch (p) {
    case Phase::AttackPhase:
        return "attack phase";
    case Phase::DrawPhase:
        return "draw phase";
    }
    return "";
}

bool haveExactName(const std::vector<CardSpecifier> &s) {
    for (const auto &c: s)
        if (c.type == CardSpecifierType::ExactName)
            return true;
    return false;
}

std::string printForEachMultiplier(const ForEachMultiplier &m) {
    std::string s;

    s += printTarget(*m.target, true);

    if (m.zone != Zone::Stage)
        s += "in your " + printZone(m.zone) + " ";

    return s;
}

