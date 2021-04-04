#include "print.h"

#include <string>

using namespace asn;

std::string printZoneChangeTrigger(const ZoneChangeTrigger &t) {
    std::string s = "When ";

    s += printTarget(t.target[0]);
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

    s += ", ";

    return s;
}

std::string printOnAttackTrigger(const OnAttackTrigger &t) {
    if (t.target.type == TargetType::ThisCard)
        return "When this card attacks, ";
    return "";
}

std::string printBattleOpponentReversedTrigger(const BattleOpponentReversedTrigger &) {
    return "When this card's battle opponent becomes" + printState(State::Reversed) + ", ";
}
std::string printOnBackupOfThis() {
    return "When you use this card's \"<b>Backup</b>\", ";
}
std::string printOnReversed() {
    return "When this card becomes" + printState(State::Reversed) + ", ";
}
std::string printPhaseTrigger(const PhaseTrigger &t) {
    std::string s;

    if (t.state == PhaseState::Start)
        s += "at the start of ";
    s += printPlayer(t.player);
    s += printPhase(t.phase) + ", ";

    return s;
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
    case TriggerType::OnBattleOpponentReversed:
        s += printBattleOpponentReversedTrigger(std::get<BattleOpponentReversedTrigger>(t.trigger));
        break;
    case TriggerType::OnPhaseEvent:
        s += printPhaseTrigger(std::get<PhaseTrigger>(t.trigger));
        break;
    case TriggerType::OnBackupOfThis:
        s += printOnBackupOfThis();
        break;
    case TriggerType::OnReversed:
        s += printOnReversed();
        break;
    }

    return s;
}
