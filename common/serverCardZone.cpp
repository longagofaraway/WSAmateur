#include "serverCardZone.h"

ServerCardZone::ServerCardZone(ServerPlayer *player, const std::string_view name)
    : mPlayer(player), mName(name) {}
