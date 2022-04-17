#include "encode.h"

using namespace asn;

void encodeZoneChangeTrigger(const ZoneChangeTrigger &t, Buf &buf) {
    encodeArray(t.target, buf, encodeTarget);
    buf.push_back(static_cast<uint8_t>(t.from));
    buf.push_back(static_cast<uint8_t>(t.to));
}

void encodeStateChangeTrigger(const StateChangeTrigger &t, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(t.state));
    encodeTarget(t.target, buf);
}

void encodePhaseTrigger(const PhaseTrigger &t, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(t.state));
    buf.push_back(static_cast<uint8_t>(t.phase));
    buf.push_back(static_cast<uint8_t>(t.player));
}

void encodeOnBeingAttacked(const OnBeingAttackedTrigger &t, Buf &buf) {
    encodeTarget(t.target, buf);
    buf.push_back(static_cast<uint8_t>(t.attackType));
}

void encodeOnDamageCancel(const OnDamageCancelTrigger &t, Buf &buf) {
    encodeTarget(t.damageDealer, buf);
    buf.push_back(t.cancelled ? 1 : 0);
}

void encodeOnDamageTakenCancel(const OnDamageTakenCancelTrigger &t, Buf &buf) {
    buf.push_back(t.cancelled ? 1 : 0);
}

void encodeOnActAbillity(const OnActAbillityTrigger &t, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(t.player));
}

void encodeTrigger(const Trigger &t, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(t.type));
    switch (t.type) {
    case TriggerType::OnZoneChange:
        encodeZoneChangeTrigger(std::get<ZoneChangeTrigger>(t.trigger), buf);
        break;
    case TriggerType::OnPhaseEvent:
        encodePhaseTrigger(std::get<PhaseTrigger>(t.trigger), buf);
        break;
    case TriggerType::OnStateChange:
        encodeStateChangeTrigger(std::get<StateChangeTrigger>(t.trigger), buf);
        break;
    case TriggerType::OnTriggerReveal:
        encodeCard(std::get<TriggerRevealTrigger>(t.trigger).card, buf);
        break;
    case TriggerType::OnPlay:
        encodeTarget(std::get<OnPlayTrigger>(t.trigger).target, buf);
        break;
    case TriggerType::OnAttack:
        encodeTarget(std::get<OnAttackTrigger>(t.trigger).target, buf);
        break;
    case TriggerType::OnBeingAttacked:
        encodeOnBeingAttacked(std::get<OnBeingAttackedTrigger>(t.trigger), buf);
        break;
    case TriggerType::OnDamageCancel:
        encodeOnDamageCancel(std::get<OnDamageCancelTrigger>(t.trigger), buf);
        break;
    case TriggerType::OnDamageTakenCancel:
        encodeOnDamageTakenCancel(std::get<OnDamageTakenCancelTrigger>(t.trigger), buf);
        break;
    case TriggerType::OnActAbillity:
        encodeOnActAbillity(std::get<OnActAbillityTrigger>(t.trigger), buf);
        break;
    case TriggerType::OtherTrigger:
        encodeString(std::get<OtherTrigger>(t.trigger).cardCode, buf);
        break;
    default:
        break;
    }
}
