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

    OtherTrigger = 255
};

struct ZoneChangeTrigger {
    std::vector<Target> target;
    Zone from;
    Zone to;
};

enum class Phase {
    NotSpecified = 0,
    Mulligan = 1,
    StandPhase = 2,
    DrawPhase = 3,
    ClockPhase = 4,
    MainPhase = 5,
    ClimaxPhase = 6,
    AttackPhase = 7,
    EndPhase = 8,
    AttackDeclarationStep = 9,
    TriggerStep = 10,
    CounterStep = 11,
    DamageStep = 12,
    BattleStep = 13,
    EncoreStep = 14
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
        OtherTrigger
    > trigger;
};

}
