#include "serverCardZone.h"

#include <algorithm>
#include <random>
#include <time.h>

ServerCardZone::ServerCardZone(ServerPlayer *player, const std::string_view name, ZoneType type)
    : mPlayer(player), mName(name), mType(type) {}

void ServerCardZone::addCard(std::shared_ptr<CardInfo> info) {
    mCards.emplace_back(std::make_unique<ServerCard>(info, this));
}

ServerCard* ServerCardZone::addCard(std::unique_ptr<ServerCard> card) {
    return mCards.emplace_back(std::move(card)).get();
}

void ServerCardZone::shuffle() {
    std::shuffle(mCards.begin(), mCards.end(), std::mt19937((unsigned int)time(0)));
}

std::unique_ptr<ServerCard> ServerCardZone::takeCard(size_t index) {
    if (mCards.size() - 1 < index )
        return {};

    auto card = std::move(mCards[index]);
    mCards.erase(mCards.begin() + index);
    return card;
}

std::unique_ptr<ServerCard> ServerCardZone::takeTopCard() {
    if (!mCards.size())
        return {};

    auto card = std::move(mCards[mCards.size() - 1]);
    mCards.pop_back();
    return card;
}
