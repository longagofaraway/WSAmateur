#include "serverCardZone.h"

#include <algorithm>
#include <random>
#include <time.h>

ServerCardZone::ServerCardZone(ServerPlayer *player, const std::string_view name)
    : mPlayer(player), mName(name) {}

void ServerCardZone::addCard(const CardInfo &info) {
    mCards.emplace_back(std::make_unique<ServerCard>(info, this));
}

void ServerCardZone::addCard(std::unique_ptr<ServerCard> card) {
    mCards.emplace_back(std::move(card));
}

void ServerCardZone::shuffle() {
    std::shuffle(mCards.begin(), mCards.end(), std::mt19937((unsigned int)time(0)));
}

std::unique_ptr<ServerCard> ServerCardZone::takeTopCard() {
    if (!mCards.size())
        return {};

    auto card = std::move(mCards[mCards.size() - 1]);
    mCards.pop_back();
    return card;
}
