#include "print.h"

using namespace asn;

extern PrintState gPrintState;
namespace {
bool isPartOfAndOr = false;
}

std::string printConditionIsCard(const ConditionIsCard &c) {
    std::string s = isPartOfAndOr ? "" : "if ";

    if (c.target.type == TargetType::BattleOpponent && c.neededCard.size() &&
        c.neededCard[0].cardSpecifiers.size() &&
        c.neededCard[0].cardSpecifiers[0].type == CardSpecifierType::LevelHigherThanOpp) {
        s += " the level of this card's battle opponent is higher than your opponent's level, ";
        gPrintState.battleOpponentMentioned = true;
        return s;
    }

    bool article = true;
    if (c.target.type == TargetType::MentionedCards ||
        c.target.type == TargetType::ChosenCards) {
        if (gPrintState.mentionedCardsNumber.value == 1)
            s += "that card is ";
        else
            s += "those cards are ";
    } else if (c.target.type == TargetType::SpecificCards) {
        if (c.target.targetSpecification->mode == TargetMode::All) {
            s += "all of ";
            s += printCard(c.target.targetSpecification->cards, true);
            s += " are ";
            article = false;
        }
    }

    for (size_t i = 0; i < c.neededCard.size(); ++i) {
        if (i)
            s += " or ";
        s += printCard(c.neededCard[i], false, article);
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
            s += "you have ";
        else if (c.who == Player::Opponent)
            s += "your opponent has ";

        if (c.howMany.mod == NumModifier::AtLeast ||
            c.howMany.mod == NumModifier::ExactMatch) {
            if (c.howMany.value == 1 && c.excludingThis) {
                s += "another ";
            } else if (c.howMany.value == 0 && c.excludingThis) {
                plural = true;
                s += "no other ";
            } else {
                plural = true;
                s += std::to_string(c.howMany.value) + " or more ";
                if (c.excludingThis)
                    s += "other ";
            }
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
