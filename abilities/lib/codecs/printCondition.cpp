#include "print.h"

using namespace asn;

std::string printConditionIsCard(const ConditionIsCard &c) {
    std::string s = "If ";

    if (c.target.type == TargetType::MentionedCards ||
        c.target.type == TargetType::ChosenCards)
        s += "that card is ";

    for (const auto &card: c.neededCard) {
        s += printCard(card, false);
    }

    return s + ", ";
}

std::string printCondition(const Condition &c) {
    std::string s;

    switch(c.type) {
    case ConditionType::IsCard:
        s += printConditionIsCard(std::get<ConditionIsCard>(c.cond));
        break;
    }

    return s;
}
