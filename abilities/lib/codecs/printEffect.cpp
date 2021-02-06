#include "print.h"

#include <cassert>
#include <string>

using namespace asn;

namespace {
    Number gChosenCardsNumber;
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

std::string printAttributeGain(const AttributeGain &e) {
    std::string res;

    switch (e.target.type) {
    case TargetType::ChosenCards:
        if (gChosenCardsNumber.mod == NumModifier::ExactMatch &&
            gChosenCardsNumber.value == 1) {
            res += "that character gets ";
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
    std::string s;

    s += "choose ";

    assert(e.targets.size() == 1);
    bool plural = false;
    const auto &target = e.targets[0];
    if (target.type == TargetType::SpecificCards) {
        const auto &spec = *target.targetSpecification;
        if (spec.number.mod == NumModifier::ExactMatch) {
            if (e.placeType == PlaceType::SpecificPlace && e.place->zone == Zone::Stage) {
                s += printDigit(spec.number.value);
                s += " of ";
                if (e.place->owner == Player::Player)
                    s += "your ";
                else if (e.place->owner == Player::Both)
                    s += "your or your opponent's ";
                plural = true;
            }
        }

        gChosenCardsNumber = spec.number;
        s += printCard(spec.cards, plural) + " ";
    }

    if (e.placeType == PlaceType::SpecificPlace && e.place->zone != Zone::Stage) {
        s += "in ";
        if (e.place->owner == Player::Player)
            s += "your ";
        s += printZone(e.place->zone) + " ";
    }

    return s;
}

std::string printRevealCard(const RevealCard &e) {
    std::string s = "reveal ";

    if (e.type == RevealType::TopDeck) {
        if (e.number.mod == NumModifier::ExactMatch) {
            if (e.number.value == 1)
                s += "the top card of your deck ";
        }
    }

    return s;
}

std::string printNonMandatory(const NonMandatory &e) {
    std::string s = "you may ";


    s += printEffects(e.effect);

    if (e.ifYouDo.size() || e.ifYouDont.size())
        s[s.size() - 1] = '.';

    s += printEffects(e.ifYouDo);
    s += printEffects(e.ifYouDont);

    return s;
}

std::string printMoveCard(const MoveCard &e) {
    std::string s = "put ";

    if (e.target.type == TargetType::ChosenCards) {
        if ((gChosenCardsNumber.mod == NumModifier::ExactMatch ||
            gChosenCardsNumber.mod == NumModifier::UpTo) &&
            gChosenCardsNumber.value == 1) {
            s += "it ";
        } else {
            s += "them ";
        }
    } else if (e.target.type == TargetType::ThisCard) {
        s += "this card ";
    } else if(e.target.type == TargetType::SpecificCards) {
        if (e.from.pos == Position::Top && e.target.targetSpecification->number.mod == NumModifier::ExactMatch &&
            e.target.targetSpecification->number.value == 1) {
            s += "the top ";
            s += printCard(e.target.targetSpecification->cards, false) + " of ";
        }
        if (e.from.owner == Player::Player)
            s += "your ";
        else
            s += "your opponent's";
        s += printZone(e.from.zone) + " ";
    }

    for (size_t i = 0; i < e.to.size(); ++i) {
        if (i)
            s += "or ";
        s += "in ";
        if (e.to[i].owner == Player::Player)
            s += "your ";
        else if (e.to[i].owner == Player::Both)
            s += "its owner's ";
        s += printZone(e.to[i].zone) + " ";
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
    case EffectType::NonMandatory:
        s += printNonMandatory(std::get<NonMandatory>(e.effect));
        break;
    case EffectType::MoveCard:
        s += printMoveCard(std::get<MoveCard>(e.effect));
        break;
    }

    return s;
}
