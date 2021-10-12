#pragma once

#include <vector>

class ServerCard;
class ServerPlayer;

enum class PlayerAttrType {
    CharAutoCannotDealDamage,
    CannotPlayEvents,
    CannotPlayBackups
};

struct PlayerAttrChange {
    // card, that gives this buff
    ServerCard *source = nullptr;
    // ability of source that gives the buff
    int abilityId = 0;
    PlayerAttrType type;
    int duration = 0;

    // constructor for effects with duration
    PlayerAttrChange(PlayerAttrType type, int duration)
        : type(type), duration(duration) {}
    // constructor for cont effect
    PlayerAttrChange(ServerCard *card, int abilityId, PlayerAttrType type)
        : source(card), abilityId(abilityId), type(type) {}
};

inline bool operator==(const PlayerAttrChange &lhs, const PlayerAttrChange &rhs) {
    return lhs.source == rhs.source && lhs.abilityId == rhs.abilityId;
}


class PlayerBuffManager {
    ServerPlayer *mPlayer;
    std::vector<PlayerAttrChange> mBuffs;

public:
    PlayerBuffManager(ServerPlayer *player) : mPlayer(player) {}

    void addAttrChange(PlayerAttrType type, int duration);
    void addContAttrChange(ServerCard *card, int abilityId, PlayerAttrType type);
    void removeContAttrChange(ServerCard *card, int abilityId, PlayerAttrType type);
    void validateAttrChanges();

    bool hasAttrChange(PlayerAttrType type) const;

    void validateBuffs();
};
