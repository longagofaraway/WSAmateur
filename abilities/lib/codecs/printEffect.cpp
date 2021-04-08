#include "print.h"

#include <cassert>
#include <string>

using namespace asn;

extern PrintState gPrintState;

std::string printEffects(const std::vector<Effect> &effects) {
    std::string s;

    for (size_t i = 0; i < effects.size(); ++i) {
        bool makeUpper = false;
        if (!gPrintState.attributeGainChaining) {
            if (i != 0 && effects[i].cond.type != ConditionType::NoCondition) {
                s[s.size() - 1] = '.';
                s.push_back(' ');
                makeUpper = true;
            } else if (i != 0) {
                if (i < effects.size() - 1) {
                    s[s.size() - 1] = ',';
                    s.push_back(' ');
                } else if (i == effects.size() - 1) {
                    s[s.size() - 1] = ',';
                    s += " and ";
                }
            }
        }
        auto effectText = printEffect(effects[i]);
        if (makeUpper)
            effectText[0] = std::toupper(effectText[0]);
        s += effectText;

        if (effects[i].type == EffectType::AttributeGain && effects.size() > i + 1 &&
            effects[i + 1].type == EffectType::AttributeGain) {
            const auto &attGain1 = std::get<AttributeGain>(effects[i].effect);
            const auto &attGain2 = std::get<AttributeGain>(effects[i + 1].effect);
            if (attGain1.target == attGain2.target)
                gPrintState.attributeGainChaining = true;
        }
    }

    return s;
}

std::string printAttributeGain(const AttributeGain &e) {
    std::string res;

    if (!gPrintState.attributeGainChaining) {
        switch (e.target.type) {
        case TargetType::ChosenCards:
            if (gPrintState.chosenCardsNumber.value == 1) {
                res += "that character gets ";
            } else if (gPrintState.chosenCardsNumber.value > 1) {
                res += "they get ";
            }
            break;
        case TargetType::ThisCard:
            res += "this card gets ";
            break;
        case TargetType::SpecificCards: {
            const auto &spec = *e.target.targetSpecification;
            bool plural = false;
            if (spec.mode == TargetMode::All || spec.mode == TargetMode::AllOther) {
                plural = true;
                res += "all of ";
            } else if (spec.mode == TargetMode::InFrontOfThis) {
                plural = true;
                res += "all of your ";
            }
            res += printCard(spec.cards, plural, true, spec.mode);
            if (spec.mode == TargetMode::InFrontOfThis)
                res += " in front of this card";
            res += " get";
            if (!plural)
                res += "s";
            res += " ";
            break;
        }
        case TargetType::OppositeThis:
            res += "the character facing this card gets ";
            break;
        default:
            assert(false);
        }
    } else {
        res += "and ";
        gPrintState.attributeGainChaining = false;
    }

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

    if (e.gainType == ValueType::Multiplier) {
        if (e.modifier->type == MultiplierType::ForEach) {
            res += "for each of ";
            res += printTarget(*e.modifier->forEach);
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
    bool article = true;
    const auto &target = e.targets[0];
    if (target.type == TargetType::SpecificCards) {
        const auto &spec = *target.targetSpecification;
        if ((spec.number.value > 1 && spec.number.mod == NumModifier::ExactMatch) ||
            (e.placeType == PlaceType::SpecificPlace && e.place->zone == Zone::Stage) ||
            spec.number.mod == NumModifier::UpTo) {
            s += printNumber(spec.number);
            if (spec.number.mod == NumModifier::UpTo)
                article = false;
        }
        if (e.placeType == PlaceType::SpecificPlace && e.place->zone == Zone::Stage) {
            s += "of " + printPlayer(e.place->owner);
            plural = true;
        }
        if (spec.mode == TargetMode::FrontRow)
            s += "center stage ";
        else if (spec.mode == TargetMode::AllOther)
            s += "other ";

        gPrintState.chosenCardsNumber = spec.number;
        s += printCard(spec.cards, plural, article) + " ";
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
        gPrintState.mentionedCardsNumber = e.number;
        if (e.number.mod == NumModifier::ExactMatch) {
            if (e.number.value == 1)
                s += "the top card of your deck ";
        }
    } else if (e.type == RevealType::ChosenCards) {
        if (gPrintState.chosenCardsNumber.value == 1)
            s += "it ";
        else
            s += "them ";
    }

    return s;
}

std::string printNonMandatory(const NonMandatory &e) {
    std::string s = "you may ";

    s += printEffects(e.effect);

    if (e.ifYouDo.size() || e.ifYouDont.size()) {
        s[s.size() - 1] = '.';
        s.push_back(' ');
    }

    if (e.ifYouDo.size()) {
        s += "If you do, ";
        s += printEffects(e.ifYouDo);

        if (e.ifYouDont.size()) {
            s[s.size() - 1] = '.';
            s.push_back(' ');
        }
    }
    if (e.ifYouDont.size()) {
        s += "If you don't, ";
        s += printEffects(e.ifYouDont);
    }

    return s;
}

std::string printMoveCard(const MoveCard &e) {
    std::string s;

    if (e.executor == Player::Opponent)
        s += "your opponent ";

    if (e.to[0].pos == Position::SlotThisWasIn)
        s += "return";
    else if (e.from.zone == Zone::Hand)
        s += "discard";
    else if (e.to[0].pos == Position::EmptySlotBackRow)
        s += "move";
    else
        s += "put";

    if (e.executor == Player::Opponent)
        s += "s";
    s += " ";


    if (e.target.type == TargetType::ChosenCards || e.target.type == TargetType::LastMovedCards) {
        if ((gPrintState.chosenCardsNumber.mod == NumModifier::ExactMatch ||
            gPrintState.chosenCardsNumber.mod == NumModifier::UpTo) &&
            gPrintState.chosenCardsNumber.value == 1) {
            s += "it ";
        } else {
            s += "them ";
        }
    } else if (e.target.type == TargetType::BattleOpponent) {
        if (gPrintState.battleOpponentMentioned)
            s += "that character ";
        else
            s += "this card's battle opponent ";
    } else if (e.target.type == TargetType::MentionedCards) {
        if (gPrintState.mentionedCardsNumber.value == 1)
            s += "it ";
        else
            s += "them ";
    } else if (e.target.type == TargetType::ThisCard) {
        s += "this card ";
    } else if(e.target.type == TargetType::SpecificCards) {
        const auto &spec = *e.target.targetSpecification;
        if (e.from.pos == Position::Top && spec.number.mod == NumModifier::ExactMatch &&
            spec.number.value == 1) {
            s += "the top ";
            s += printCard(spec.cards, false, false) + " of ";
        } else {
            bool plural = false;
            if (spec.number.value > 1 || spec.number.mod == NumModifier::UpTo) {
                if (spec.number.value > 1)
                    plural = true;
                s += printNumber(spec.number);
            }
            s += printCard(spec.cards, plural) + " from ";
        }
        s += printPlayer(e.from.owner, e.executor);
        s += printZone(e.from.zone) + " ";
    }

    for (size_t i = 0; i < e.to.size(); ++i) {
        if (i)
            s += "or ";

        if (e.to[i].pos == Position::EmptySlotBackRow)
            return s + "to an empty slot in the back stage ";
        if (e.to[i].pos == asn::Position::SlotThisWasIn) {
            s += "to its previous position as" + printState(asn::State::Rested);
            return s;
        }
        if (e.to[i].zone == Zone::Stage) {
            s += "on ";
            if (e.to[i].pos == Position::NotSpecified)
                s += "any position of ";
        } else if (e.to[i].zone == Zone::Deck) {
            if (e.to[i].pos == Position::Top)
                s += "on the top of ";
        } else {
            s += "into ";
        }
        if (e.to[i].owner == Player::Player)
            s += "your ";
        else if (e.to[i].owner == Player::Both)
            s += "its owner's ";
        else if (e.to[i].owner == Player::Opponent &&
            (e.executor == Player::Opponent || e.target.type == TargetType::SpecificCards))
            s += "their ";
        else if (e.to[i].owner == Player::Opponent)
            s += "your opponent's ";
        s += printZone(e.to[i].zone) + " ";

        if (e.order == Order::Any)
            s += "in any order ";
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
    gPrintState.chosenCardsNumber = e.targets[0].number;

    bool plural = false;
    if (e.targets[0].number.value > 1)
        plural = true;
    s += printCard(e.targets[0].cards[0], plural, false) + " ";

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

std::string printMoveWrToDeck(const MoveWrToDeck &e) {
    std::string s;
    if (e.executor == asn::Player::Both)
        s += "all players return all cards in their waiting room to their deck, and all players shuffle their decks ";

    return s;
}

std::string printFlipOver(const FlipOver &e) {
    std::string s;

    s += "Flip over " + std::to_string(e.number.value);
    s += " cards from the top of your deck, and put them into your waiting room. ";
    s += "For each " + printCard(e.forEach, false, false) + " revealed, ";
    s += printEffects(e.effect);

    return s;
}

std::string printChangeState(const ChangeState &e) {
    std::string s;

    s += printState(e.state);
    s += printTarget(e.target);

    return s;
}

std::string printBackup(const Backup &e) {
    std::string s;

    s += "<b>" + std::to_string(e.power) + ", Level " + std::to_string(e.level) + "</b> ";

    return s;
}

std::string printTriggerCheckTwice() {
    return "during that attack, perform a trigger check 2 times on the trigger step ";
}

std::string printLook(const Look &e) {
    std::string s = "look at ";

    if (e.number.mod != NumModifier::UpTo)
        s += "the top ";

    if (!(e.number.mod == NumModifier::ExactMatch && e.number.value == 1))
        s += printNumber(e.number);

    s += "card";
    if (e.number.value > 1)
        s += "s";
    s += " ";

    if (e.number.mod != NumModifier::UpTo)
        s += "of ";
    else
        s += "from the top of ";

    s += printPlayer(e.place.owner);
    s += printZone(e.place.zone) + " ";

    return s;
}

std::string printEarlyPlay() {
    return "this card gets -1 level while in your hand ";
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
    case EffectType::MoveWrToDeck:
        s += printMoveWrToDeck(std::get<MoveWrToDeck>(e.effect));
        break;
    case EffectType::FlipOver:
        s += printFlipOver(std::get<FlipOver>(e.effect));
        break;
    case EffectType::ChangeState:
        s += printChangeState(std::get<ChangeState>(e.effect));
        break;
    case EffectType::Backup:
        s += printBackup(std::get<Backup>(e.effect));
        break;
    case EffectType::TriggerCheckTwice:
        s += printTriggerCheckTwice();
        break;
    case EffectType::Look:
        s += printLook(std::get<Look>(e.effect));
        break;
    case EffectType::EarlyPlay:
        s += printEarlyPlay();
        break;
    }

    return s;
}
