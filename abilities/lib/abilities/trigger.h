#pragma once

#include "basicTypes.h"
#include "target.h"

enum class TriggerType : uint8_t {
    OnZoneChange = 1,
    OnPlay,
    OnReversed,
    OnAttack,
    OnBattleOpponentReversed,
    OnBackupOfThis,
    OnTriggerReveal,
    OnPhaseEvent,
    OnEndOfThisCardsAttack,
    OnOppCharPlacedByStandbyTriggerReveal,
    OnEndOfThisTurn
};

struct ZoneChangeTrigger {
    std::vector<Target> target;
    AsnZone from;
    AsnZone to;
};

enum class AsnPhase {
    DrawPhase = 1,
    ClimaxPhase,
    AttackPhase
};

enum class PhaseState {
    Start = 1,
    End
};

struct PhaseTrigger {
    AsnPhase phase;
    PhaseState state;
    AsnPlayer player;
};

struct TriggerRevealTrigger {
    AsnCard card;
};

struct OnPlayTrigger {
    Target target;
};
struct OnAttackTrigger {
    Target target;
};

struct BattleOpponentReversedTrigger {
    AsnCard card;
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
        BattleOpponentReversedTrigger
    > trigger;
};
