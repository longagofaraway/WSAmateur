#include "print.h"

#include <array>
#include <cassert>
#include <string>
#include <unordered_map>

using namespace asn;

extern PrintState gPrintState;

namespace {
std::unordered_map<std::string, std::string> gOtherEffects {
    { "KGL/S79-020-3", "Both players reveal the top card of their deck. If your revealed card's level is higher than \
opponent's revealed card's level, choose 1 " + printTrait("Shuchiin") + " character from your waiting room and \
return it to hand. If opponent's revealed card's level is higher, opponent chooses 1 of their characters, \
and that character gets +5000 power until end of turn" }
};

bool canChain(EffectType type) {
    std::array<EffectType, 2> arr = { EffectType::AttributeGain, EffectType::AbilityGain };
    return std::find(arr.begin(), arr.end(), type) != arr.end();
}

bool compareChainTargets(const Effect& effect1, const Effect& effect2) {
    const Target *t1,*t2;
    if (effect1.type == EffectType::AttributeGain)
        t1 = &std::get<AttributeGain>(effect1.effect).target;
    else
        t1 = &std::get<AbilityGain>(effect1.effect).target;

    if (effect2.type == EffectType::AttributeGain)
        t2 = &std::get<AttributeGain>(effect2.effect).target;
    else
        t2 = &std::get<AbilityGain>(effect2.effect).target;
    return *t1 == *t2;
}
}

std::string printEffects(const std::vector<Effect> &effects) {
    std::string s;

    for (size_t i = 0; i < effects.size(); ++i) {
        bool makeUpper = false;
        if (!gPrintState.abilityChainingSecond) {
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

        bool chaining = false;
        if (canChain(effects[i].type) && effects.size() > i + 1 &&
             canChain(effects[i + 1].type)) {
            chaining = true;
            if (compareChainTargets(effects[i], effects[i + 1]))
                gPrintState.abilityChainingFirst = true;
        }

        auto effectText = printEffect(effects[i]);
        if (makeUpper)
            effectText[0] = std::toupper(effectText[0]);
        s += effectText;

        if (chaining) {
            if (compareChainTargets(effects[i], effects[i + 1]))
                gPrintState.abilityChainingSecond = true;
        }
    }

    return s;
}

std::string printAttributeGain(const AttributeGain &e) {
    std::string res;

    if (!gPrintState.abilityChainingSecond) {
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
            break;
        }
    } else {
        res += "and ";
        gPrintState.abilityChainingSecond = false;
    }

    if (e.value > 0)
        res += "+";

    if (e.gainType == ValueType::Multiplier && e.modifier->type == MultiplierType::TimesLevel)
        res += "X";
    else
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

    if (e.gainType == ValueType::Multiplier && e.modifier->type == MultiplierType::ForEach) {
        res += "for each ";
        res += printForEachMultiplier(*e.modifier->specifier);
    }

    if (!gPrintState.abilityChainingFirst) {
        res += printDuration(e.duration);
        gPrintState.abilityChainingFirst = false;
    }

    if (e.gainType == ValueType::Multiplier && e.modifier->type == MultiplierType::TimesLevel) {
        res.pop_back();
        res += ". X is equal to " + std::to_string(e.value) + " multiplied by that character's level ";
    }

    return res;
}

std::string printChooseCard(const ChooseCard &e) {
    std::string s;

    s += "choose ";

    assert(e.targets.size() == 1);
    bool plural = false;
    bool article = true;
    const auto &target = e.targets[0];
    if (target.target.type == TargetType::SpecificCards) {
        const auto &spec = *target.target.targetSpecification;
        if ((spec.number.value > 1 && spec.number.mod == NumModifier::ExactMatch) ||
            (target.placeType == PlaceType::SpecificPlace && target.place->zone == Zone::Stage) ||
            spec.number.mod == NumModifier::UpTo) {
            s += printNumber(spec.number);
            if (spec.number.mod == NumModifier::UpTo)
                article = false;
        }
        if (target.placeType == PlaceType::SpecificPlace && target.place->zone == Zone::Stage) {
            s += "of " + printPlayer(target.place->owner);
            plural = true;
        }
        if (spec.mode == TargetMode::FrontRow)
            s += "center stage ";
        else if (spec.mode == TargetMode::AllOther)
            s += "other ";

        gPrintState.chosenCardsNumber = spec.number;
        s += printCard(spec.cards, plural, article) + " ";
    } else if (target.target.type == TargetType::CharInBattle) {
        s += "one of your characters in battle ";
        gPrintState.battleOpponentMentioned = true;
        gPrintState.chosenCardsNumber = Number{NumModifier::ExactMatch, 1};
    }

    if (target.placeType == PlaceType::SpecificPlace && target.place->zone != Zone::Stage) {
        s += "in " + printPlayer(target.place->owner);
        s += printZone(target.place->zone) + " ";
    }

    return s;
}

std::string printRevealCard(const RevealCard &e) {
    std::string s = "reveal ";

    if (e.type == RevealType::TopDeck) {
        gPrintState.mentionedCardsNumber = e.number;
        if (e.number.mod == NumModifier::ExactMatch) {
            s += "the top ";
            if (e.number.value > 1)
                s += std::to_string(e.number.value) + " ";
            s += "card";
            if (e.number.value > 1)
                s += "s";
            s += " of your deck ";
        } else if (e.number.mod == NumModifier::UpTo) {
            s += printNumber(e.number);
            s += "card";
            if (e.number.value > 1)
                s += "s";
            s += " from the top of your deck ";
        }
    } else if (e.type == RevealType::ChosenCards) {
        if (gPrintState.chosenCardsNumber.value == 1)
            s += "it ";
        else
            s += "them ";
    } else if (e.type == RevealType::FromHand) {
        s += printCard(*e.card) + " from your hand ";
    }

    return s;
}

std::string printNonMandatory(const NonMandatory &e) {
    gPrintState.mandatory = false;
    assert(e.effect.size());
    std::string player;
    Player executor = Player::Player;
    if (e.effect[0].type == EffectType::MoveCard)
        executor = std::get<MoveCard>(e.effect[0].effect).executor;

    std::string s;
    if (executor == Player::Player)
        s = "you may ";
    else
        s = "your opponent may ";

    s += printEffects(e.effect);
    gPrintState.mandatory = true;

    if (e.ifYouDo.size() || e.ifYouDont.size()) {
        s[s.size() - 1] = '.';
        s.push_back(' ');
    }

    if (e.ifYouDo.size()) {
        if (executor == Player::Player)
            s += "If you do, ";
        else
            s += "If your opponent does, ";
        s += printEffects(e.ifYouDo);

        if (e.ifYouDont.size()) {
            s[s.size() - 1] = '.';
            s.push_back(' ');
        }
    }
    if (e.ifYouDont.size()) {
        if (executor == Player::Player)
            s += "If you don't, ";
        else
            s += "If your opponent doesn't, ";
        s += printEffects(e.ifYouDont);
    }

    return s;
}

std::string printMoveCard(const MoveCard &e) {
    std::string s;

    if (e.executor == Player::Opponent && gPrintState.mandatory)
        s += "your opponent ";

    if (e.to[0].pos == Position::SlotThisWasIn ||
        e.to[0].pos == Position::SlotThisWasInRested)
        s += "return";
    else if (e.from.zone == Zone::Hand)
        s += "discard";
    else if (e.to[0].pos == Position::EmptySlotBackRow ||
             e.to[0].pos == Position::EmptySlotFrontRow ||
             e.to[0].pos == Position::EmptySlot)
        s += "move";
    else
        s += "put";

    if (e.executor == Player::Opponent && gPrintState.mandatory)
        s += "s";
    s += " ";


    if (e.target.type == TargetType::RestOfTheCards) {
        s += "the rest ";
    } else if (e.target.type == TargetType::ChosenCards || e.target.type == TargetType::LastMovedCards) {
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
        if (gPrintState.mentionedCardsNumber.value <= 1)
            s += "it ";
        else
            s += "them ";
    } else if (e.target.type == TargetType::ThisCard) {
        s += "this card ";
    } else if(e.target.type == TargetType::SpecificCards) {
        bool plural = false;
        const auto &spec = *e.target.targetSpecification;
        // 'of' case
        if ((e.from.pos == Position::Top ||e.from.pos == Position::Bottom)
                && spec.number.mod == NumModifier::ExactMatch) {
            if (e.from.pos == Position::Top)
                s += "the top ";
            else if (e.from.pos == Position::Bottom)
                s += "the bottom ";
            if (spec.number.value > 1) {
                plural = true;
                s += std::to_string(spec.number.value) + " ";
            }
            s += printCard(spec.cards, plural, false) + " of ";
        } else {
            // 'from' case
            bool plural = false;
            if (spec.mode == TargetMode::All) {
                plural = true;
                s += "all ";
            } else if (spec.number.value > 1 || spec.number.mod == NumModifier::UpTo) {
                if (spec.number.value > 1)
                    plural = true;
                s += printNumber(spec.number);
            }
            s += printCard(spec.cards, plural) + " from ";
        }
        s += printPlayer(e.from.owner, e.executor);
        s += printZone(e.from.zone) + " ";
    } else if (e.target.type == TargetType::MentionedInTrigger) {
        s += "that character ";
    }

    for (size_t i = 0; i < e.to.size(); ++i) {
        if (i)
            s += "or ";

        if (e.to[i].pos == Position::EmptySlotBackRow)
            return s + "to an empty slot in the back stage ";
        else if (e.to[i].pos == Position::EmptySlotFrontRow)
            return s + "to an empty slot in the center stage ";
        else if (e.to[i].pos == Position::EmptySlot)
            return s + "to an empty slot of " + printPlayer(e.to[i].owner) + "stage ";
        else if (e.to[i].pos == Position::SlotThisWasIn ||
                 e.to[i].pos == Position::SlotThisWasInRested) {
            if (e.target.type == asn::TargetType::ThisCard)
                s += "to its previous position ";
            else
                s += "on the stage position that this card was on ";
            if (e.to[i].pos == Position::SlotThisWasInRested)
                return s + "as" + printState(State::Rested) + " ";
            else
                return s;
        }

        if (e.to[i].zone == Zone::Stage) {
            s += "on ";
            if (e.to[i].pos == Position::NotSpecified)
                s += "any position of ";
        } else if (e.to[i].zone == Zone::Deck) {
            if (e.to[i].pos == Position::Top)
                s += "on the top of ";
            else if (e.to[i].pos == Position::Bottom)
                s += "on the bottom of ";
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
    if (e.value.mod == NumModifier::ExactMatch) {
        if (e.value.value == 1)
            s += "a card ";
        else
            s += std::to_string(e.value.value) + " cards ";
    } else {
        s += printNumber(e.value);
        s += "card";
        if (e.value.value > 1)
            s += "s";
        s += " ";
    }
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
    std::string s;

    if (!gPrintState.abilityChainingSecond) {
        s += printTarget(e.target, false, true);
        s += "get";
        if ((e.target.type == TargetType::ChosenCards &&
            gPrintState.chosenCardsNumber.value == 1) ||
            e.target.type == TargetType::ThisCard)
            s += "s";
        s += " ";
    } else {
        s += "and ";
        gPrintState.abilityChainingSecond = false;
    }

    if (static_cast<size_t>(e.number) == e.abilities.size()) {
        s += "the following ";
        if (e.number == 1)
            s += "ability ";
        else
            s += std::to_string(e.number) + " abilities ";
    } else {
        s += std::to_string(e.number) + " of the following " + std::to_string(e.abilities.size()) + " abilities of your choice ";
    }

    s += printDuration(e.duration);
    s.pop_back();
    s += ". ";

    for (const auto &a: e.abilities)
        s += "\"" + printAbility(a) + "\" ";
    s.pop_back();
    return s;
}

std::string printMoveWrToDeck(const MoveWrToDeck &e) {
    std::string s;
    if (e.executor == Player::Both)
        s += "each player returns all cards in the waiting room to their deck, and each player shuffles their deck ";
    else if (e.executor == Player::Player)
        s += "return all cards in your waiting room to your deck, and shuffle your deck ";

    return s;
}

std::string printFlipOver(const FlipOver &e) {
    std::string s;

    s += "flip over " + std::to_string(e.number.value);
    s += " cards from the top of your deck, and put them into your waiting room. ";
    s += "For each " + printCard(e.forEach, false, false) + " revealed, ";
    s += printEffects(e.effect);

    return s;
}

std::string printChangeState(const ChangeState &e) {
    std::string s;

    bool plural = false;
    s += printState(e.state) + " ";
    if (e.target.type == TargetType::SpecificCards) {
        const auto &spec = *e.target.targetSpecification;
        if (spec.number.value > 1 || spec.mode == TargetMode::AllOther) {
            plural = true;
            s += std::to_string(spec.number.value) + " of ";
        }
    }
    s += printTarget(e.target, plural);

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

    gPrintState.mentionedCardsNumber = e.number;
    if (e.number.mod != NumModifier::UpTo)
        s += "the top ";

    if (!(e.number.mod == NumModifier::ExactMatch && e.number.value == 1) &&
        e.valueType != ValueType::Multiplier)
        s += printNumber(e.number);
    if (e.valueType == ValueType::Multiplier) {
        if (e.number.mod == NumModifier::UpTo)
            s += "up to ";
        s += "X ";
    }

    s += "card";
    if (e.number.value > 1 || e.valueType == ValueType::Multiplier)
        s += "s";
    s += " ";

    if (e.number.mod != NumModifier::UpTo)
        s += "of ";
    else
        s += "from the top of ";

    s += printPlayer(e.place.owner);
    s += printZone(e.place.zone) + " ";

    if (e.valueType == ValueType::Multiplier && e.multiplier.value().type == MultiplierType::ForEach) {
        s += "(X is equal to the number of ";
        s += printForEachMultiplier(e.multiplier.value().specifier.value(), true);
        s.pop_back();
        s += ") ";
    }

    return s;
}

std::string printEarlyPlay() {
    return "this card gets -1 level while in your hand ";
}
std::string printCannotPlay() {
    return "this card cannot be played from your hand ";
}

std::string printPerformEffect(const PerformEffect &e) {
    std::string s;

    if (e.numberOfEffects < static_cast<int>(e.effects.size())) {
        s += "choose " + std::to_string(e.numberOfEffects) + " of the following " + std::to_string(e.effects.size()) + " effects and perform ";
        if (e.numberOfEffects == 1)
            s += "it";
        else
            s += "them";
        s += ".";
    } else {
        s += "perform the following effect";
        if (e.numberOfEffects > 1)
            s += "s";
        if (e.numberOfTimes == 2)
            s += " twice";
        else if (e.numberOfTimes == 3)
            s += " three times";
        s += ".";
    }

    for (const auto &ab: e.effects) {
        s += "<br>\"";
        s += printSpecificAbility(ab, CardType::Char);
        s += "\"";
    }

    return s;
}

std::string printDealDamage(const DealDamage &e) {
    std::string s = "deal ";

    if (e.damageType == ValueType::Multiplier)
        s += "X";
    else
        s += std::to_string(e.damage);

    s += " damage to your opponent";

    if (e.damageType == ValueType::Multiplier) {
        s += ". X is equal to ";
        if (e.modifier->type == MultiplierType::ForEach)
            s += "the number " + printForEachMultiplier(*e.modifier->specifier, true);
    } else
        s += " ";

    return s;
}

std::string printCannotUseBackupOrEvent(const CannotUseBackupOrEvent &e) {
    std::string s;

    if (e.player == Player::Player)
        s += "you ";
    else if (e.player == Player::Opponent)
        s += "your opponent ";
    else
        s += "you and your opponent ";

    s += "cannot play ";

    switch (e.what) {
    case BackupOrEvent::Backup:
        s += "\"Backup\"";
        break;
    case BackupOrEvent::Event:
        s += "event cards";
        break;
    case BackupOrEvent::Both:
        s += "event cards or \"Backup\"";
        break;
    }

    s += " from hand ";

    return s;
}

std::string printSwapCards(const SwapCards &e) {
    std::string s;

    s += printChooseCard(e.first) + "and ";
    auto text2 = printChooseCard(e.second);
    text2.erase(0, 7);
    s += text2;

    s += "and swap them ";

    return s;
}

std::string printCannotAttack(const CannotAttack &e) {
    std::string s;

    s = printTarget(e.target);
    s += "cannot " + printAttackType(e.type) + " ";
    s += printDuration(e.duration);

    return s;
}

std::string printCannotBecomeReversed(const CannotBecomeReversed &e) {
    std::string s;

    s = printTarget(e.target);
    s += "cannot become " + printState(State::Reversed);
    s += printDuration(e.duration);

    return s;
}

std::string printOpponentAutoCannotDealDamage(const OpponentAutoCannotDealDamage &e) {
    assert(e.duration == 0);
    return "you cannot take damage from your opponent character's 【AUTO】 effects ";
}

std::string printStockSwap() {
    return "put all of your opponent's stock into your opponent's waiting room, and"
           " your opponent puts the same number of cards from the top of their deck into the stock ";
}

std::string printCannotMove(const CannotMove &e) {
    std::string s;

    s += printDuration(e.duration);
    s += printTarget(e.target);
    s += "cannot move to another position on the stage ";

    return s;
}

std::string printSideAttackWithoutPenalty(const SideAttackWithoutPenalty &e) {
    std::string s;

    s += printTarget(e.target);
    s.pop_back();
    s += "'s soul does not decrease by side attacking ";
    s += printDuration(e.duration);

    return s;
}

std::string printPutOnStageRested(const PutOnStageRested &e) {
    std::string s;

    MoveCard m;
    m.target = e.target;
    m.from = e.from;
    m.executor = Player::Player;
    m.order = Order::NotSpecified;
    m.to.emplace_back(Place{e.to, Zone::Stage, Player::Player});

    s += printMoveCard(m);
    s += "as" + printState(State::Rested);

    return s;
}

std::string printOtherEffect(const OtherEffect &e) {
    return gOtherEffects[e.cardCode + '-' + std::to_string(e.effectId)];
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
    case EffectType::CannotPlay:
        s += printCannotPlay();
        break;
    case EffectType::PerformEffect:
        s += printPerformEffect(std::get<PerformEffect>(e.effect));
        break;
    case EffectType::DealDamage:
        s += printDealDamage(std::get<DealDamage>(e.effect));
        break;
    case EffectType::CannotUseBackupOrEvent:
        s += printCannotUseBackupOrEvent(std::get<CannotUseBackupOrEvent>(e.effect));
        break;
    case EffectType::SwapCards:
        s += printSwapCards(std::get<SwapCards>(e.effect));
        break;
    case EffectType::CannotAttack:
        s += printCannotAttack(std::get<CannotAttack>(e.effect));
        break;
    case EffectType::CannotBecomeReversed:
        s += printCannotBecomeReversed(std::get<CannotBecomeReversed>(e.effect));
        break;
    case EffectType::OpponentAutoCannotDealDamage:
        s += printOpponentAutoCannotDealDamage(std::get<OpponentAutoCannotDealDamage>(e.effect));
        break;
    case EffectType::StockSwap:
        s += printStockSwap();
        break;
    case EffectType::CannotMove:
        s += printCannotMove(std::get<CannotMove>(e.effect));
        break;
    case EffectType::SideAttackWithoutPenalty:
        s += printSideAttackWithoutPenalty(std::get<SideAttackWithoutPenalty>(e.effect));
        break;
    case EffectType::PutOnStageRested:
        s += printPutOnStageRested(std::get<PutOnStageRested>(e.effect));
        break;
    case EffectType::OtherEffect:
        s += printOtherEffect(std::get<OtherEffect>(e.effect));
        break;
    }

    return s;
}
