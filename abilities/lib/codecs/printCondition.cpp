#include "print.h"

using namespace asn;

namespace {
bool isPartOfAndOr = false;
}

std::string printConditionIsCard(const ConditionIsCard &c) {
    std::string s = isPartOfAndOr ? "" : "if ";

    bool article = true;
    if (c.target.type == TargetType::MentionedCards ||
        c.target.type == TargetType::ChosenCards)
        s += "that card is ";
    else if (c.target.type == TargetType::SpecificCards) {
        if (c.target.targetSpecification->mode == TargetMode::All) {
            s += "all of ";
            s += printCard(c.target.targetSpecification->cards, true);
            s += " are ";
            article = false;
        }
    }

    for (const auto &card: c.neededCard) {
        s += printCard(card, false, article);
    }

    if (!isPartOfAndOr)
        s += ", ";
    return s;
}

std::string printConditionHaveCard(const ConditionHaveCard &c) {
    std::string s = isPartOfAndOr ? "" : "if ";

    bool plural = false;
    if (haveExactName(c.whichCards.cardSpecifiers)) {
        s += "a card named ";
    } else {
        if (c.who == Player::Player)
            s += "you ";

        s += "have ";
        if ((c.howMany.mod == NumModifier::AtLeast ||
            c.howMany.mod == NumModifier::ExactMatch) &&
            c.howMany.value == 1 && c.excludingThis)
            s += "another ";
        else if (c.howMany.mod == NumModifier::AtLeast) {
            plural = true;
            s += std::to_string(c.howMany.value) + " or more ";
            if (c.excludingThis)
                s += "other ";
        }
    }

    s += printCard(c.whichCards, plural, false);

    if (c.where.zone != Zone::Stage) {
        if (haveExactName(c.whichCards.cardSpecifiers)) {
            s += " is ";
        }
        s += "in ";
        if (c.where.owner == Player::Player)
            s += "your ";
        s += printZone(c.where.zone);
    }

    if (s.back() == ' ')
        s.pop_back();
    if (!isPartOfAndOr)
        s += ", ";
    return s;
}

std::string printConditionAnd(const ConditionAnd &c) {
    std::string s = "if ";
    isPartOfAndOr = true;

    for (size_t i = 0; i < c.cond.size(); ++i) {
        s += printCondition(c.cond[i]);
        if (i != c.cond.size() - 1)
            s += " and ";
    }

    isPartOfAndOr = false;
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
    case ConditionType::And:
        s += printConditionAnd(std::get<ConditionAnd>(c.cond));
        break;
    }

    return s;
}
