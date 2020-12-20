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

    size_t count() const { return mCards.size(); }
    const std::string& name() const { return mName; }
    ZoneType type() const { return mType; }
    void addCard(std::shared_ptr<CardInfo> info);
    void addCard(size_t pos);
    ServerCard* addCard(std::unique_ptr<ServerCard> card);
    std::unique_ptr<ServerCard> swapCards(std::unique_ptr<ServerCard> card, size_t pos);
    void swapCards(size_t pos1, size_t pos2);
    std::unique_ptr<ServerCard> takeCard(size_t index);
    std::unique_ptr<ServerCard> takeTopCard();
    std::unique_ptr<ServerCard> takeCardFromPos(size_t pos);
    ServerCard* card(size_t index);
    bool hasCardWithColor(char color) const;
    void shuffle();
};
