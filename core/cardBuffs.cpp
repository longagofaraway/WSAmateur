#include "serverPlayer.h"

#include "abilityEvents.pb.h"
#include "gameEvent.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"

void ServerPlayer::endOfTurnEffectValidation() {
    mBuffManager.validateAttrChanges();

    auto stage = zone("stage");
    for (int i = 0; i < 5; ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        card->setTriggerCheckTwice(false);

        card->buffManager()->endOfTurnEffectValidation();
    }
    mAttacksThisTurn = 0;
}

void ServerPlayer::removePositionalContBuffsBySource(ServerCard *source) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        card->buffManager()->removePositionalContBuffsBySource(source);
    }
}
