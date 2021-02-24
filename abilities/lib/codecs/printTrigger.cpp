#include "print.h"

#include <string>

using namespace asn;

std::string printZoneChangeTrigger(const ZoneChangeTrigger &t) {
    std::string s = "When ";

    s += printTarget(t.target[0]);
    s += "is placed on ";
    if (t.from == Zone::NotSpecified)
        s += "your ";
    s += printZone(t.to);

    if (t.from != Zone::NotSpecified) {
        s += "from your " + printZone(t.from);
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
    }

    return s;
}
