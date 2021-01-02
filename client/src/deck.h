#pragma once

#include "cardModel.h"
#include "cardZone.h"

class Player;
class Game;

class Deck: public CardZone
{
    Player *mPlayer;
    Game *mGame;
    CardModel mCardsModel;
    QQuickItem *mQmlObject;

public:
    Deck(Player *player, Game *game);

    QQuickItem* visualItem() const override { return mQmlObject; }
    std::vector<Card> & cards() override { return mCardsModel.cards(); }
    void removeCard(int index) override { mCardsModel.removeCard(index); }
    CardModel & model() override { return mCardsModel; }
};
