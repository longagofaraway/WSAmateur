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

const Card* CardZone::findCardById(int id) const {
    for (const auto &card: mCardsModel.cards())
        if (card.id() == id)
            return &card;
    return nullptr;
}

void CardZone::fixedHighlightByAbility(int cardId) {
    auto &cards = mCardsModel.cards();
    for (size_t i = 0; i < cards.size(); ++i) {
        if (cards[i].id() == cardId) {
            mCardsModel.setHighlightedByAbility(i, true);
            cards[i].fixHighlightByAbility(true);
        }
    }
}

void CardZone::unhighlightByAbility(int cardId) {
    auto &cards = mCardsModel.cards();
    for (size_t i = 0; i < cards.size(); ++i) {
        if (cards[i].id() == cardId) {
            cards[i].fixHighlightByAbility(false);
            mCardsModel.setHighlightedByAbility(i, false);
        }
    }
}
