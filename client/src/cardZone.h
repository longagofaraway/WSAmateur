#pragma once

#include <vector>

#include "card.h"
#include "cardModel.h"

class QQuickItem;

class CardModel;

class CardZone
{
protected:
    CardModel mCardsModel;

public:
    virtual ~CardZone() {}

    virtual QQuickItem* visualItem() const = 0;
    virtual int numOfSelectedCards();
    virtual int numOfHighlightedCards();
    virtual std::vector<Card>& cards() { return mCardsModel.cards(); }
    virtual CardModel& model() { return mCardsModel; }
    virtual void removeCard(int index) { mCardsModel.removeCard(index); }
};
