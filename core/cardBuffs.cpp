#include "serverPlayer.h"

#include "abilityEvents.pb.h"
#include "gameEvent.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"

void ServerPlayer::endOfTurnEffectValidation() {
    mBuffManager.validateAttrChanges();

    for (const auto &zoneIt: zones()) {
        const auto zone = zoneIt.second.get();
        for (int i = 0; i < zone->count(); ++i) {
            auto card = zone->card(i);
            if (!card)
                continue;

            card->setTriggerCheckTwice(false);
            card->setFirstTurn(false);

            card->buffManager()->endOfTurnEffectValidation();
        }
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
