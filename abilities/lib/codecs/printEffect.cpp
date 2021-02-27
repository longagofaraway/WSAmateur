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
    case TargetType::ThisCard:
        res += "this card gets ";
        break;
    default:
        assert(false);
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
        res += "until end of turn ";
    else if (e.duration == 2)
        res += "until end of your opponent's turn ";

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
                s += " of " + printPlayer(e.place->owner);
                plural = true;
            }
        }
        if (spec.mode == TargetMode::FrontRow)
            s += "center stage ";

        gChosenCardsNumber = spec.number;
        s += printCard(spec.cards, plural) + " ";
    }

    if (e.placeType == PlaceType::SpecificPlace && e.place->zone != Zone::Stage) {
        s += "in " + printPlayer(e.place->owner);
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
    } else if (e.type == RevealType::ChosenCards) {
        if (gChosenCardsNumber.value == 1)
            s += "it ";
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
    std::string s;
    if (e.to[0].pos == asn::Position::SlotThisWasIn)
        s += "return ";
    else
        s += "put ";

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
        s += printPlayer(e.from.owner);
        s += printZone(e.from.zone) + " ";
    }

    for (size_t i = 0; i < e.to.size(); ++i) {
        if (i)
            s += "or ";

        if (e.to[i].pos == asn::Position::SlotThisWasIn) {
            s += "to its previous position as" + printState(asn::State::Rested);
            return s;
        }
        s += "into ";
        if (e.to[i].owner == Player::Player)
            s += "your ";
        else if (e.to[i].owner == Player::Both)
            s += "its owner's ";
        else if (e.to[i].owner == Player::Opponent)
            s += "your opponent's ";
        s += printZone(e.to[i].zone) + " ";
    }

    return s;
}

std::string printDrawCard(const DrawCard &e) {
    std::string s = "draw ";
    if (e.value.mod == NumModifier::ExactMatch && e.value.value == 1)
        s += "a card ";
    return s;
}

std::string printPayCost(const PayCost &e) {
    std::string s = "you may pay the cost. ";

    if (e.ifYouDo.size()) {
        s += "If you do, ";
        s += printEffects(e.ifYouDo);
    }

    if (e.ifYouDont.size()) {
        s += "If you don't, ";
        s += printEffects(e.ifYouDont);
    }

    return s;
}

std::string printSearchCard(const SearchCard &e) {
    std::string s = "search ";

    if (e.place.owner == Player::Player)
        s += "your ";
    s += printZone(e.place.zone) + " for ";
    assert(e.targets.size() == 1);
    assert(e.targets[0].cards.size() == 1);
    s += printNumber(e.targets[0].number);
    gChosenCardsNumber = e.targets[0].number;
    s += printCard(e.targets[0].cards[0], false, false) + " ";

    return s;
}

std::string printShuffle(const Shuffle &e) {
    std::string s = "shuffle ";

    if (e.owner == Player::Player)
        s += "your ";
    s += printZone(e.zone);

    return s;
}

std::string printAbilityGain(const AbilityGain &e) {
    std::string s = printTarget(e.target);

    s += "gets ";
    if (static_cast<size_t>(e.number) == e.abilities.size()) {
        s += "the following ";
        if (e.number == 1)
            s += "ability ";
        else
            s += std::to_string(e.number) + " abilities ";
    } else {
        s += std::to_string(e.number) + " of the following " + std::to_string(e.abilities.size()) + " abilities of your choice ";
    }

    if (e.duration == 1)
        s += "until end of turn. ";
    else if (e.duration == 2)
        s += "until end of your opponent's turn. ";

    for (const auto &a: e.abilities)
        s += "\"" + printAbility(a) + "\" ";
    s.pop_back();
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
    case EffectType::DrawCard:
        s += printDrawCard(std::get<DrawCard>(e.effect));
        break;
    case EffectType::PayCost:
        s += printPayCost(std::get<PayCost>(e.effect));
        break;
    case EffectType::SearchCard:
        s += printSearchCard(std::get<SearchCard>(e.effect));
        break;
    case EffectType::Shuffle:
        s += printShuffle(std::get<Shuffle>(e.effect));
        break;
    case EffectType::AbilityGain:
        s += printAbilityGain(std::get<AbilityGain>(e.effect));
        break;
    }

    return s;
}
