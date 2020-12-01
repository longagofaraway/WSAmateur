#pragma once

#include <string>
#include <vector>

class ServerPlayer;
class ServerCard;

class ServerCardZone
{
    ServerPlayer *mPlayer;
    std::string mName;
    std::vector<std::shared_ptr<ServerCard>> mCards;
public:
    ServerCardZone(ServerPlayer *player, const std::string_view name);

    const std::string& name() const { return mName; }
};
