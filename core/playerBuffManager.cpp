#include "playerBuffManager.h"

#include "serverPlayer.h"


void PlayerBuffManager::addAttrChange(PlayerAttrType type, int duration) {
    mBuffs.emplace_back(type, duration);
    mPlayer->changeAttribute(type, true);
}

void PlayerBuffManager::addContAttrChange(ServerCard *card, int abilityId, PlayerAttrType type) {
    PlayerAttrChange change(card, abilityId, type);
    auto it = std::find(mBuffs.begin(), mBuffs.end(), change);
    if (it != mBuffs.end())
        return;

    mBuffs.emplace_back(std::move(change));
    mPlayer->changeAttribute(type, true);
    return;
}

void PlayerBuffManager::removeContAttrChange(ServerCard *card, int abilityId, PlayerAttrType type) {
    PlayerAttrChange change(card, abilityId, type);
    auto it = std::find(mBuffs.begin(), mBuffs.end(), change);
    if (it != mBuffs.end()) {
        mBuffs.erase(it);
        if (!hasAttrChange(type))
            mPlayer->changeAttribute(type, false);
    }
}

void PlayerBuffManager::validateAttrChanges() {
    auto it = mBuffs.begin();
    while (it != mBuffs.end()) {
        if (!it->duration || --it->duration) {
            ++it;
            continue;
        }
        auto type = it->type;
        it = mBuffs.erase(it);
        if (!hasAttrChange(type)) {
            mPlayer->changeAttribute(type, false);
        }
    }
}

bool PlayerBuffManager::hasAttrChange(PlayerAttrType type) const {
    auto sameType = std::find_if(mBuffs.begin(), mBuffs.end(),
                                 [type](const PlayerAttrChange &el) { return el.type == type; });
    return sameType != mBuffs.end();
}
