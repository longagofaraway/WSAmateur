#pragma once

#include <vector>

#include "card.h"
#include "cardModel.h"

class QQuickItem;

class CardModel;
class Player;

class CardZone
{
protected:
    Player *mPlayer;
    CardModel mCardsModel;
    std::string mName;

public:
    CardZone(Player *player) : mPlayer(player) {}
    virtual ~CardZone() {}

    Player* player() { return mPlayer; }

    virtual const std::string& name() const { return mName; }
    virtual QQuickItem* visualItem() const = 0;
    virtual int numOfSelectedCards();
    virtual int numOfHighlightedCards();
    virtual std::vector<Card>& cards() { return mCardsModel.cards(); }
    virtual CardModel& model() { return mCardsModel; }
    virtual void removeCard(int index) { mCardsModel.removeCard(index); }
    const Card* findCardById(int id) const;

    void fixedHighlightByAbility(int cardId);
    void unhighlightByAbility(int cardId);
};
