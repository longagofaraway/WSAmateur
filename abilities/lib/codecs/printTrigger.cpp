#include "print.h"

#include <algorithm>
#include <string>
#include <unordered_map>

using namespace asn;

extern PrintState gPrintState;

namespace {
std::unordered_map<std::string, std::string> gOtherTriggers {
    { "KGL/S79-016", "When your opponent uses \"<b>Brainstorm</b>\" and that effect puts at least 1 climax\
 card in their waiting room, " }
};

bool hasExactName(const Target &target) {
    if (target.type != TargetType::SpecificCards)
        return false;
    const auto &spec = target.targetSpecification.value();
    const auto &cardTraits = spec.cards.cardSpecifiers;
    return std::any_of(cardTraits.begin(), cardTraits.end(),
                       [](const CardSpecifier &c){
        return c.type == CardSpecifierType::ExactName;
    });
}

bool triggersOnMoveToTheSamePlace(const std::vector<asn::Trigger> &triggers) {
    if (!(triggers[0].type == asn::TriggerType::OnZoneChange &&
        triggers[0].type == triggers[1].type))
        return false;
    const auto &trigger = std::get<asn::ZoneChangeTrigger>(triggers[1].trigger);

    gPrintState.doubleZoneChangeTrigger = printZone(trigger.from);
    return true;
}
}

std::string printTriggers(const std::vector<asn::Trigger> &triggers) {
    std::string s;

    for (size_t i = 0; i < triggers.size(); ++i) {
        if (i > 0 && s.size() > 2) {
            s.pop_back();
            s.pop_back();
            s += " or ";
        }

        bool needToBreak = false;
        if (triggers.size() >= 2 && i == 0) {
            if (triggersOnMoveToTheSamePlace(triggers)) {
                needToBreak = true;
            }
        }

        s += printTrigger(triggers[i]);
        if (needToBreak)
            break;
    }

    return s;
}

std::string printZoneChangeTrigger(const ZoneChangeTrigger &t) {
    std::string s = "when ";

    for (size_t i = 0; i < t.target.size(); ++i) {
        if (i)
            s += "or ";
        s += printTarget(t.target[i]);
    }
    s += "is placed ";
    if (t.to == Zone::Stage)
        s += "on ";
    else
        s += "into ";
    if (t.to != Zone::Stage)
        s += "your ";
    s += printZone(t.to);

    if (t.from != Zone::NotSpecified) {
        s += " from ";
        if (t.from == Zone::Stage)
            s += "the stage";
        else
            s += "your " + printZone(t.from);
    }
    if (!gPrintState.doubleZoneChangeTrigger.empty()) {
        s += " or " + gPrintState.doubleZoneChangeTrigger;
    }

    s += ", ";

    return s;
}

std::string printOnAttackTrigger(const OnAttackTrigger &t) {
    if (t.target.type == TargetType::ThisCard)
        return "when this card attacks, ";
    return "";
}

std::string printStateChangeTrigger(const StateChangeTrigger &t) {
    std::string s = "when ";

    s += printTarget(t.target);

    s += "becomes" + printState(t.state) + ", ";

    return s;
}

std::string printOnBackupOfThis() {
    return "when you use this card's \"<b>Backup</b>\", ";
}
std::string printOnReversed() {
    return "when this card becomes" + printState(State::Reversed) + ", ";
}
std::string printPhaseTrigger(const PhaseTrigger &t) {
    std::string s;

    if (t.phase == Phase::EndPhase)
        return "at the end of turn, ";
    if (t.state == PhaseState::Start)
        s += "at the start of ";
    else
        s += "at the end of ";
    s += printPlayer(t.player);
    s += printPhase(t.phase) + ", ";

    return s;
}

std::string printOnTriggerReveal(const TriggerRevealTrigger &t) {
    std::string s = "when your character's trigger check reveals ";

    s += printCard(t.card) + ", ";

    return s;
}

std::string printOnPlay(const OnPlayTrigger &t) {
    std::string s = "when you play ";

    s += printTarget(t.target);
    s.pop_back();
    s += ", ";

    return s;
}

std::string printEndOfAttack() {
    return "at the end of this card's attack, ";
}

std::string printOnOppCharPlacedByStanbyTrigger() {
    std::string s = "when your opponent puts a character on the stage by the effect of ";
    s += printTriggerIcon(TriggerIcon::Standby) + " trigger, ";
    return s;
}

std::string printOnBeingAttackedTrigger(const OnBeingAttackedTrigger &t) {
    std::string s = "when ";

    s += printTarget(t.target);
    s += "is" + printAttackType(t.attackType) + " attacked, ";

    return s;
}

std::string printOnDamageCancelTrigger(const OnDamageCancelTrigger &t) {
    std::string s = "when damage dealt by ";

    s += printTarget(t.damageDealer) + "is ";
    if (!t.cancelled)
        s += "not ";
    s += "cancelled, ";

    return s;
}

std::string printOnDamageTakenCancelTrigger(const OnDamageTakenCancelTrigger &t) {
    std::string s = "when damage taken by you is ";
    if (!t.cancelled)
        s += "not ";
    s += "cancelled, ";
    return s;
}

std::string printOnPayingCost(const OnPayingCostTrigger &t) {
    std::string s = "when you pay the cost of ";

    if (hasExactName(t.target))
        s += printAbilityType(t.abilityType) + " of ";
    s += printTarget(t.target);
    return s;
}

std::string printOnActAbillityTrigger(const OnActAbillityTrigger &t) {
    std::string s = "when ";
    if (t.player == Player::Player)
        s += "you use";
    else if (t.player == Player::Opponent)
            s += "your opponent uses";
    s += " 【ACT】 abillity, ";
    return s;
}

std::string printOtherTrigger(const OtherTrigger &t) {
    return gOtherTriggers[t.cardCode];
}

std::string printTrigger(const Trigger &t) {
    std::string s;

    switch (t.type) {
    case TriggerType::OnZoneChange:
        s += printZoneChangeTrigger(std::get<ZoneChangeTrigger>(t.trigger));
        break;
    case TriggerType::OnAttack:
        s += printOnAttackTrigger(std::get<OnAttackTrigger>(t.trigger));
        break;
    case TriggerType::OnStateChange:
        s += printStateChangeTrigger(std::get<StateChangeTrigger>(t.trigger));
        break;
    case TriggerType::OnPhaseEvent:
        s += printPhaseTrigger(std::get<PhaseTrigger>(t.trigger));
        break;
    case TriggerType::OnBackupOfThis:
        s += printOnBackupOfThis();
        break;
    case TriggerType::OnTriggerReveal:
        s += printOnTriggerReveal(std::get<TriggerRevealTrigger>(t.trigger));
        break;
    case TriggerType::OnPlay:
        s += printOnPlay(std::get<OnPlayTrigger>(t.trigger));
        break;
    case TriggerType::OnEndOfThisCardsAttack:
        s += printEndOfAttack();
        break;
    case TriggerType::OnOppCharPlacedByStandbyTriggerReveal:
        s += printOnOppCharPlacedByStanbyTrigger();
        break;
    case TriggerType::OnBeingAttacked:
        s += printOnBeingAttackedTrigger(std::get<OnBeingAttackedTrigger>(t.trigger));
        break;
    case TriggerType::OnDamageCancel:
        s += printOnDamageCancelTrigger(std::get<OnDamageCancelTrigger>(t.trigger));
        break;
    case TriggerType::OnDamageTakenCancel:
        s += printOnDamageTakenCancelTrigger(std::get<OnDamageTakenCancelTrigger>(t.trigger));
        break;
    case TriggerType::OnPayingCost:
        s += printOnPayingCost(std::get<OnPayingCostTrigger>(t.trigger));
        break;
    case TriggerType::OnActAbillity:
        s += printOnActAbillityTrigger(std::get<OnActAbillityTrigger>(t.trigger));
        break;
    case TriggerType::OtherTrigger:
        s += printOtherTrigger(std::get<OtherTrigger>(t.trigger));
        break;
    default:
        break;
    }

    return s;
}
