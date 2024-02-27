#include "quickSetters.h"

#include "../hardcodedAbilities.h"

asn::Trigger createTrigger(QString triggerName) {
    if (triggerName == "Placed from hand to stage") {
        return fromHandToStagePreset();
    }
    if (triggerName == "When this card attacks") {
        return thisCardAttacks();
    }
    if (triggerName == "When this becomes reversed") {
        return thisCardBecomesReversed();
    }
    if (triggerName == "Climax placed") {
        return climaxIsPlaced();
    }
    if (triggerName == "Start of opp's attack phase") {
        return startOfOppAttackPhase();
    }
    assert(false);
    return {};
}
