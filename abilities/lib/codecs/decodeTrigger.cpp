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

StateChangeTrigger decodeStateChangeTrigger(Iterator &it, Iterator end) {
    StateChangeTrigger t;

    t.state = decodeEnum<State>(it, end);
    t.target = decodeTarget(it, end);

    return t;
}

OnBeingAttackedTrigger decodeOnBeingAttackedTrigger(Iterator &it, Iterator end) {
    OnBeingAttackedTrigger t;

    t.target = decodeTarget(it, end);
    t.attackType = decodeEnum<AttackType>(it, end);

    return t;
}

OnDamageCancelTrigger decodeOnDamageCancelTrigger(Iterator &it, Iterator end) {
    OnDamageCancelTrigger t;

    t.damageDealer = decodeTarget(it, end);
    t.cancelled = decodeBool(it, end);

    return t;
}

OnDamageTakenCancelTrigger decodeOnDamageTakenCancelTrigger(Iterator &it, Iterator end) {
    OnDamageTakenCancelTrigger t;

    t.cancelled = decodeBool(it, end);

    return t;
}

OnPayingCostTrigger decodeOnPayingCostTrigger(Iterator &it, Iterator end) {
    OnPayingCostTrigger t;

    t.abilityType = decodeEnum<AbilityType>(it, end);
    t.target = decodeTarget(it, end);

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
    case TriggerType::OnStateChange:
        t.trigger = decodeStateChangeTrigger(it, end);
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
    case TriggerType::OnBeingAttacked:
        t.trigger = decodeOnBeingAttackedTrigger(it, end);
        break;
    case TriggerType::OnDamageCancel:
        t.trigger = decodeOnDamageCancelTrigger(it, end);
        break;
    case TriggerType::OnDamageTakenCancel:
        t.trigger = decodeOnDamageTakenCancelTrigger(it, end);
        break;
    case TriggerType::OnPayingCost:
        t.trigger = decodeOnPayingCostTrigger(it, end);
        break;
    case TriggerType::OtherTrigger:
        t.trigger = OtherTrigger{ decodeString(it, end) };
        break;
    default:
        break;
    }

    return t;
}
