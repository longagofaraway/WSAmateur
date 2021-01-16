#include "print.h"

#include <string>

namespace {
    Number gChosenCardsNumber;
}

std::string printAttributeGain(const AttributeGain &e) {
    std::string res;

    switch (e.target.type) {
    case TargetType::ChosenCards:
        if (gChosenCardsNumber.mod == NumModifier::ExactMatch &&
            gChosenCardsNumber.value == 1) {
            res += "and that character gets ";
        }
        break;
    }

    if (e.gainType == ValueType::Raw) {
        if (e.value > 0)
            res += "+";
        res += std::to_string(e.value);
        switch (e.type) {
        case AttributeType::Power:
            res += " power ";
            break;
        case AttributeType::Soul:
            res += " soul ";
            break;
        case AttributeType::Level:
            res += " level ";
            break;
        }
    }

    if (e.duration == 1)
        res += "until end of turn.";
    else if (e.duration == 2)
        res += "until end of your opponent's turn.";

    return res;
}

std::string printChooseCard(const ChooseCard &e) {
    std::string res;

    res += "choose ";

    if (e.targets.size() == 1) {
        const auto &target = e.targets[0];
        if (target.type == TargetType::SpecificCards) {
            const auto &spec = *target.targetSpecification;
            if (spec.number.mod == NumModifier::ExactMatch) {
                res += printDigit(spec.number.value);
                res += " of ";
                gChosenCardsNumber = spec.number;
            }

            res += printCard(spec.cards, true);

            res += ", ";
        }
    }

    return res;
}

std::string printRevealCard(const RevealCard &e) {
    std::string s = "Reveal ";

    if (e.type == RevealType::TopDeck) {
        if (e.number.mod == NumModifier::ExactMatch) {
            if (e.number.value == 1)
                s += "the top card of your deck. ";
        }
    }

    return s;
}

std::string printEffect(const Effect &e) {
    std::string s;

    if (e.cond.type != ConditionType::NoCondition)
        s += printCondition(e.cond);

    switch (e.type) {
    case EffectType::ChooseCard:
        s += printChooseCard(std::get<ChooseCard>(e.effect));
        break;
    case EffectType::AttributeGain:
        s += printAttributeGain(std::get<AttributeGain>(e.effect));
        break;
    case EffectType::RevealCard:
        s += printRevealCard(std::get<RevealCard>(e.effect));
        break;
    }

    return s;
}
