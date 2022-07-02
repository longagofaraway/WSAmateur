#include "abilityPlayer.h"

#include "serverPlayer.h"

Resumable AbilityPlayer::playOtherEffect(const asn::OtherEffect &e) {
    auto key = e.cardCode + '-' + std::to_string(e.effectId);
    if (key == "KGL/S79-020-3") {
        co_await playS79_20();
    } else if (key == "DBG/W87-053-2") {
        co_await playW87_53();
    }
}
