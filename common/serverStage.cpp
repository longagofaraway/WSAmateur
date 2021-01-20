#include "serverStage.h"

ServerStage::ServerStage(ServerPlayer *player)
    : ServerCardZone(player, "stage", ZoneType::PublicZone)
{
    for (int i = 0; i < 5; ++i)
        mCards.emplace_back(nullptr);
}

ServerCard *ServerStage::addCard(std::unique_ptr<ServerCard>) {
    assert(false);
    return nullptr;
}

std::unique_ptr<ServerCard> ServerStage::takeCard(int index) {
    if (static_cast<size_t>(index) >= mCards.size())
        return {};

    auto card = std::move(mCards[index]);
    mCards[index].reset(nullptr);
    return card;
}

std::unique_ptr<ServerCard> ServerStage::putOnStage(std::unique_ptr<ServerCard> card, int pos) {
    if (!card)
        return {};

    std::swap(mCards[pos], card);
    mCards[pos]->setPos(pos);
    return card;
}

void ServerStage::switchPositions(int pos1, int pos2) {
    std::swap(mCards[pos1], mCards[pos2]);
    if (mCards[pos1])
        mCards[pos1]->setPos(pos1);
    if (mCards[pos2])
        mCards[pos2]->setPos(pos2);
}
