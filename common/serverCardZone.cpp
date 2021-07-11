#include "serverCardZone.h"

#include <algorithm>
#include <random>
#include <time.h>

ServerCardZone::ServerCardZone(ServerPlayer *player, const std::string_view name, ZoneType type)
    : mPlayer(player), mName(name), mType(type) {}

void ServerCardZone::addCard(std::shared_ptr<CardInfo> info, int uniqueId) {
    auto addedCard = mCards.emplace_back(std::make_unique<ServerCard>(info, this, uniqueId)).get();
    addedCard->setPos(static_cast<int>(mCards.size()) - 1);
    addedCard->setZone(this);
}

ServerCard* ServerCardZone::addCard(std::unique_ptr<ServerCard> card) {
    auto addedCard = mCards.emplace_back(std::move(card)).get();
    addedCard->setPos(static_cast<int>(mCards.size()) - 1);
    addedCard->setZone(this);
    return addedCard;
}

std::unique_ptr<ServerCard> ServerCardZone::putOnStage(std::unique_ptr<ServerCard>, int) {
    assert(false);
    return {};
}

void ServerCardZone::switchPositions(int pos1, int pos2) {
    std::swap(mCards[pos1], mCards[pos2]);
    if (mCards[pos1])
        mCards[pos1]->setPos(pos1);
    if (mCards[pos2])
        mCards[pos2]->setPos(pos2);
}

void ServerCardZone::shuffle() {
    std::shuffle(mCards.begin(), mCards.end(), std::mt19937((unsigned int)time(0)));
    resetPositions();
}

void ServerCardZone::resetPositions() {
    for (size_t i = 0; i < mCards.size(); ++i)
        mCards[i]->setPos(static_cast<int>(i));
}

std::unique_ptr<ServerCard> ServerCardZone::takeCard(int index) {
    if (static_cast<size_t>(index) >= mCards.size())
        return {};

    auto card = std::move(mCards[index]);
    mCards.erase(mCards.begin() + index);
    resetPositions();
    return card;
}

std::unique_ptr<ServerCard> ServerCardZone::takeTopCard() {
    if (!mCards.size()) {
        assert(false);
        return {};
    }

    auto card = std::move(mCards[mCards.size() - 1]);
    mCards.pop_back();
    return card;
}

ServerCard *ServerCardZone::card(int index) {
    if (static_cast<size_t>(index) >= mCards.size())
        return nullptr;

    return mCards[index].get();
}

ServerCard *ServerCardZone::topCard() {
    if (mCards.empty())
        return nullptr;

    return mCards[mCards.size() - 1].get();
}

bool ServerCardZone::hasCardWithColor(char color) const {
    for (auto &card: mCards) {
        if (card->color() == color)
            return true;
    }

    return false;
}
