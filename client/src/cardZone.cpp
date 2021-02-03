#include "cardZone.h"

int CardZone::numOfSelectedCards() {
    int num = 0;
    for (const auto &card: mCardsModel.cards())
        if (card.selected())
            num++;

    return num;
}
