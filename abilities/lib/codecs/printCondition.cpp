#include "print.h"

using namespace asn;

extern PrintState gPrintState;
namespace {
bool isPartOfAndOr = false;
bool firstInHaveCardChain = true;

bool haveCardChain(int current, std::vector<Condition> vc) {
    // check if the next condition 'HaveCards' describes the same
    if (vc.size() <= current + 1)
        return false;

    if (vc[current].type != ConditionType::HaveCards ||
        vc[current + 1].type != ConditionType::HaveCards)
        return false;

    auto &c1 = std::get<ConditionHaveCard>(vc[current].cond);
    auto &c2 = std::get<ConditionHaveCard>(vc[current + 1].cond);
    return c1.where.zone == c2.where.zone;
}
}

std::string printConditionIsCard(const ConditionIsCard &c) {
    std::string s = "if ";

    if (c.target.type == TargetType::BattleOpponent && c.neededCard.size() &&
        c.neededCard[0].cardSpecifiers.size()) {
        gPrintState.battleOpponentMentioned = true;
        const auto &spec = c.neededCard[0].cardSpecifiers[0];
        if (spec.type == CardSpecifierType::LevelHigherThanOpp)
            s += "the level of this card's battle opponent is higher than your opponent's level, ";
        else if (spec.type == CardSpecifierType::Cost ||
                 spec.type == CardSpecifierType::Level) {
            std::string t = "of this card's battle opponent is ";
            if (spec.type == CardSpecifierType::Cost) {
                s += "the cost " + t;
                s += printNumber(std::get<CostSpecifier>(spec.specifier).value, true);
            } else if (spec.type == CardSpecifierType::Level) {
                s += "the level " + t;
                s += printNumber(std::get<Level>(spec.specifier).value, true);
            }

            s.pop_back();
            s += ", ";
        }

        return s;
    }

    bool article = true;
    if (c.target.type == TargetType::ThisCard) {
        s += "this card is ";
    } else if (c.target.type == TargetType::MentionedCards ||
        c.target.type == TargetType::ChosenCards) {
        if (gPrintState.mentionedCardsNumber.value == 1)
            s += "that card is ";
        else
            s += "those cards are ";
    } else if (c.target.type == TargetType::LastMovedCards) {
        s += "that card is ";
    } else if (c.target.type == TargetType::SpecificCards) {
        if (c.target.targetSpecification->mode == TargetMode::All) {
            s += "all of ";
            s += printCard(c.target.targetSpecification->cards, true);
            s += " are ";
            article = false;
        }
    } else if (c.target.type == TargetType::OppositeThis) {
        s += printTarget(c.target) + "is ";
        article = false;
    }

    for (size_t i = 0; i < c.neededCard.size(); ++i) {
        if (i)
            s += " or ";
        s += printCard(c.neededCard[i], false, article);
    }

    s += ", ";
    return s;
}

namespace {
std::string playerHas(asn::Player p) {
    if (p == Player::Player)
        return "you have ";
    else if (p == Player::Opponent)
        return "your opponent has ";
    else return "";
}
}
std::string printConditionHaveCard(const ConditionHaveCard &c) {
    std::string s = "if ";

    bool plural = false;
    bool article = false;

    if (c.invert) {
        if (c.howMany.mod == NumModifier::AtLeast &&
            c.howMany.value == 1) {
            s += "you do not have ";
        }

        s += printCard(c.whichCards) + ", ";

        return s;
    } else if (haveExactName(c.whichCards.cardSpecifiers)) {
        if (c.where.zone == Zone::Stage && firstInHaveCardChain)
            s += playerHas(c.who);
        s += "a card named ";
    } else {
        if (c.who == Player::Player)
            s += "you have ";
        else if (c.who == Player::Opponent)
            s += "your opponent has ";

        if (c.howMany.value == 1 && c.excludingThis) {
            s += "another ";
        } else if (c.howMany.value == 0 && c.excludingThis) {
            plural = true;
            s += "no other ";
        } else {
            if (!(c.where.zone == Zone::Stage && c.howMany.value == 1)) {
                plural = true;
                s += std::to_string(c.howMany.value) + " ";
            } else {
                article = true;
            }
            if (c.howMany.mod == NumModifier::AtLeast &&
                !(c.where.zone == Zone::Stage && c.howMany.value == 1))
                s += "or more ";
            else if (c.howMany.mod == NumModifier::UpTo)
                s += "or less ";
            if (c.excludingThis)
                s += "other ";
        }
    }

    s += printCard(c.whichCards, plural, article) + " ";

    if (c.where.zone != Zone::Stage) {
        if (haveExactName(c.whichCards.cardSpecifiers)) {
            s += "is ";
        }
        s += "in ";
        if (c.where.owner == Player::Player)
            s += "your ";
        s += printZone(c.where.zone);
    }

    if (s.back() == ' ')
        s.pop_back();
    s += ", ";
    if (firstInHaveCardChain)
        firstInHaveCardChain = false;
    return s;
}

std::string printConditionAnd(const ConditionAnd &c) {
    std::string s;
    isPartOfAndOr = true;
    firstInHaveCardChain = true;

    bool firstIf = true;
    for (size_t i = 0; i < c.cond.size(); ++i) {
        auto condStr = printCondition(c.cond[i]);
        if (condStr.starts_with("if ")) {
            if (i > 0 && !firstIf)
                condStr.erase(0, 3);
            firstIf = false;
        }
        s += condStr;
        if (c.cond[i].type != ConditionType::DuringTurn) {
            if (s.ends_with(", "))
                s.erase(s.size() - 2, 2);
            if (i != c.cond.size() - 1)
                s += " and ";
        }

        firstInHaveCardChain = !haveCardChain(i, c.cond);
    }

    isPartOfAndOr = false;
    return s + ", ";
}

std::string printConditionInBatteWithThis() {
    return "in battles involving this card, ";
}

std::string printConditionSumOfLevels(const ConditionSumOfLevels &c) {
    std::string s = "if the total level of cards in your level is ";
    s += std::to_string(c.equalOrMoreThan) + " or higher, ";
    return s;
}

std::string printConditionDuringTurn(const ConditionDuringTurn &c) {
    return "during " + printPlayer(c.player) + "turn, ";
}

std::string printConditionCheckMilledCards(const ConditionCheckMilledCards &c) {
    std::string s = "if there is ";

    s += printNumber(c.number);
    s += printCard(c.card, c.number.value > 1, false);
    s += " among those cards, ";

    return s;
}

std::string printCardsLocation(const ConditionCardsLocation &c) {
    std::string s = "if ";

    s += printTarget(c.target);
    s += "is in ";
    s += printPlace(c.place) + ", ";

    return s;
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
    case ConditionType::InBattleWithThis:
        s += printConditionInBatteWithThis();
        break;
    case ConditionType::SumOfLevels:
        s += printConditionSumOfLevels(std::get<ConditionSumOfLevels>(c.cond));
        break;
    case ConditionType::DuringTurn:
        s += printConditionDuringTurn(std::get<ConditionDuringTurn>(c.cond));
        break;
    case ConditionType::CheckMilledCards:
        s += printConditionCheckMilledCards(std::get<ConditionCheckMilledCards>(c.cond));
        break;
    case ConditionType::CardsLocation:
        s += printCardsLocation(std::get<ConditionCardsLocation>(c.cond));
        break;
    default:
        break;
    }

    return s;
}
