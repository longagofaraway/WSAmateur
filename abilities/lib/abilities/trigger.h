#pragma once

#include "basicTypes.h"
#include "target.h"

namespace asn {

enum class TriggerType : uint8_t {
    NotSpecified = 0,
    OnZoneChange,
    OnPlay,
    OnStateChange,
    OnAttack,
    OnBackupOfThis,
    OnTriggerReveal,
    OnPhaseEvent,
    OnEndOfThisCardsAttack,
    OnOppCharPlacedByStandbyTriggerReveal,
    OnBeingAttacked,
    OnDamageCancel,
    OnDamageTakenCancel,
    OnPayingCost,
    OnActAbillity,

    OtherTrigger = 255
};

struct ZoneChangeTrigger {
    std::vector<Target> target;
    Zone from;
    Zone to;
};

enum class PhaseState {
    Start = 1,
    End
};

struct PhaseTrigger {
    Phase phase;
    PhaseState state;
    Player player;
};

struct TriggerRevealTrigger {
    Card card;
};

struct OnPlayTrigger {
    Target target;
};
struct OnAttackTrigger {
    Target target;
};

struct StateChangeTrigger {
    State state;
    Target target;
};

struct OnBeingAttackedTrigger {
    Target target;
    AttackType attackType;
};

struct OnDamageCancelTrigger {
    Target damageDealer;
    bool cancelled;
};

struct OnDamageTakenCancelTrigger {
    bool cancelled;
};
struct OnActAbillityTrigger {
    Player player;
};

struct OnPayingCostTrigger {
    AbilityType abilityType;
    Target target;
};

struct OtherTrigger {
    std::string cardCode;
};

struct Trigger {
    TriggerType type;
    std::variant<
        std::monostate,
        ZoneChangeTrigger,
        PhaseTrigger,
        TriggerRevealTrigger,
        OnPlayTrigger,
        OnAttackTrigger,
        StateChangeTrigger,
        OnBeingAttackedTrigger,
        OnDamageCancelTrigger,
        OnDamageTakenCancelTrigger,
        OnPayingCostTrigger,
        OnActAbillityTrigger,
        OtherTrigger
    > trigger;
};

}
