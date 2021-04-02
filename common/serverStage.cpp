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
    if (index >= static_cast<int>(mCards.size()) || index < 0)
        return {};

    auto card = std::move(mCards[index]);
    card->setPrevStagePos(card->pos());
    mCards[index].reset(nullptr);
    return card;
}

std::unique_ptr<ServerCard> ServerStage::putOnStage(std::unique_ptr<ServerCard> card, int pos) {
    if (!card)
        return {};

    std::swap(mCards[pos], card);
    mCards[pos]->setPos(pos);
    mCards[pos]->setZone(this);
    return card;
}
