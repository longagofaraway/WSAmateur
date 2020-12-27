#pragma once

#include <string>
#include <vector>

#include "serverCard.h"

class CardInfo;
class ServerPlayer;

enum class ZoneType {
    PublicZone,
    HiddenZone,
    PrivateZone
};

class ServerCardZone
{
    ServerPlayer *mPlayer;
    std::string mName;
    ZoneType mType;
    std::vector<std::unique_ptr<ServerCard>> mCards;

public:
    ServerCardZone(ServerPlayer *player, const std::string_view name, ZoneType type);

    int count() const { return static_cast<int>(mCards.size()); }
    const std::string& name() const { return mName; }
    ZoneType type() const { return mType; }
    void addCard(std::shared_ptr<CardInfo> info);
    void addCard(int pos);
    ServerCard* addCard(std::unique_ptr<ServerCard> card);
    std::unique_ptr<ServerCard> swapCards(std::unique_ptr<ServerCard> card, int pos);
    void swapCards(int pos1, int pos2);
    std::unique_ptr<ServerCard> takeCard(int index);
    std::unique_ptr<ServerCard> takeTopCard();
    std::unique_ptr<ServerCard> takeCardFromPos(int pos);
    ServerCard* card(int index);
    bool hasCardWithColor(char color) const;
    void shuffle();
};
