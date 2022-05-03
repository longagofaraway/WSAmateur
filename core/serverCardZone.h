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
protected:
    ServerPlayer *mPlayer;
    std::string mName;
    ZoneType mType;
    std::vector<std::unique_ptr<ServerCard>> mCards;

public:
    ServerCardZone(ServerPlayer *player, const std::string_view name, ZoneType type);
    virtual ~ServerCardZone(){}

    ServerPlayer* player() const { return mPlayer; }

    int count() const { return static_cast<int>(mCards.size()); }
    const std::string& name() const { return mName; }
    ZoneType type() const { return mType; }

    void addCard(std::shared_ptr<CardInfo> info, int uniqueId);
    virtual ServerCard* addCard(std::unique_ptr<ServerCard> card, int targetPos = -1);
    virtual std::unique_ptr<ServerCard> putOnStage(std::unique_ptr<ServerCard> card, int pos);
    virtual void switchPositions(int pos1, int pos2);
    virtual std::unique_ptr<ServerCard> takeCard(int index);
    std::unique_ptr<ServerCard> takeTopCard();
    ServerCard* card(int index);
    ServerCard* topCard();
    bool hasCardWithColor(char color) const;
    void shuffle();
    void resetPositions(int from = 0);
};
