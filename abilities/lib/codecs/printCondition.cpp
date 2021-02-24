#include "print.h"

using namespace asn;

std::string printConditionIsCard(const ConditionIsCard &c) {
    std::string s = "if ";

    if (c.target.type == TargetType::MentionedCards ||
        c.target.type == TargetType::ChosenCards)
        s += "that card is ";
    else if (c.target.type == TargetType::SpecificCards) {
        if (c.target.targetSpecification->mode == TargetMode::All) {
            s += "all of ";
            s += printCard(c.target.targetSpecification->cards, true);
            s += " are ";
        }
    }

    for (const auto &card: c.neededCard) {
        s += printCard(card, false);
    }

    return s + ", ";
}

std::string printConditionHaveCard(const ConditionHaveCard &c) {
    std::string s = "if ";

    if (c.who == Player::Player)
        s += "you ";

    s += "have ";
    if ((c.howMany.mod == NumModifier::AtLeast ||
        c.howMany.mod == NumModifier::ExactMatch) &&
        c.howMany.value == 1 && c.excludingThis)
        s += "another ";

    s += printCard(c.whichCards, false, false);

    return s + ", ";
}

std::string printCondition(const Condition &c) {
    std::string s;

    switch(c.type) {
    case ConditionType::IsCard:
        s += printConditionIsCard(std::get<ConditionIsCard>(c.cond));
        break;
    case ConditionType::HaveCards:
        s += printConditionHaveCard(std::get<ConditionHaveCard>(c.cond));
        break;
    }

    return s;
}
