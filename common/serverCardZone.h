#pragma once

#include <string>
#include <vector>

#include "serverCard.h"

class CardInfo;
class ServerPlayer;

class ServerCardZone
{
    ServerPlayer *mPlayer;
    std::string mName;
    std::vector<std::unique_ptr<ServerCard>> mCards;
public:
    ServerCardZone(ServerPlayer *player, const std::string_view name);

    const std::string& name() const { return mName; }
    void addCard(const CardInfo &info);
    void addCard(std::unique_ptr<ServerCard> card);
    std::unique_ptr<ServerCard> takeTopCard();
    void shuffle();
};
