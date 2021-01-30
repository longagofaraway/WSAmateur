#pragma once

#include "basicTypes.h"
#include "target.h"

namespace asn {

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
    Zone from;
    Zone to;
};

enum class Phase {
    DrawPhase = 1,
    ClimaxPhase,
    AttackPhase
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

struct BattleOpponentReversedTrigger {
    Card card;
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

}
