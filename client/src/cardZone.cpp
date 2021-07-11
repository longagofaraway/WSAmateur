#include "cardZone.h"

int CardZone::numOfSelectedCards() {
    int num = 0;
    for (const auto &card: mCardsModel.cards())
        if (card.selected())
            num++;

    return num;
}

int CardZone::numOfHighlightedCards() {
    int num = 0;
    for (const auto &card: mCardsModel.cards())
        if (card.glow())
            num++;

    return num;
}

const Card* CardZone::findCardById(int id) const
{
    for (const auto &card: mCardsModel.cards())
        if (card.id() == id)
            return &card;
    return nullptr;
}
