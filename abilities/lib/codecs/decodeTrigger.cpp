#include "decode.h"

using namespace asn;

PhaseTrigger decodePhaseTrigger(Iterator &it, Iterator end) {
    PhaseTrigger t;

    t.state = decodeEnum<PhaseState>(it, end);
    t.phase = decodeEnum<Phase>(it, end);
    t.player = decodeEnum<Player>(it, end);

    return t;
}

ZoneChangeTrigger decodeZoneChangeTrigger(Iterator &it, Iterator end) {
    ZoneChangeTrigger t;

    t.target = decodeArray<Target>(it, end, decodeTarget);
    t.from = decodeEnum<Zone>(it, end);
    t.to = decodeEnum<Zone>(it, end);

    return t;
}

Trigger decodeTrigger(Iterator &it, Iterator end) {
    Trigger t;

    t.type = decodeEnum<TriggerType>(it, end);
    switch (t.type) {
    case TriggerType::OnZoneChange:
        t.trigger = decodeZoneChangeTrigger(it, end);
        break;
    case TriggerType::OnPhaseEvent:
        t.trigger = decodePhaseTrigger(it, end);
        break;
    case TriggerType::OnBattleOpponentReversed:
        t.trigger = BattleOpponentReversedTrigger{ decodeCard(it, end) };
        break;
    case TriggerType::OnTriggerReveal:
        t.trigger = TriggerRevealTrigger{ decodeCard(it, end) };
        break;
    case TriggerType::OnPlay:
        t.trigger = OnPlayTrigger{ decodeTarget(it, end) };
        break;
    case TriggerType::OnAttack:
        t.trigger = OnAttackTrigger{ decodeTarget(it, end) };
        break;
    default:
        break;
    }

    return t;
}
