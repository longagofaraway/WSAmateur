#pragma once

#include "cardModel.h"
#include "cardZone.h"

class Player;
class Game;

class CommonCardZone: public CardZone
{
    Player *mPlayer;
    Game *mGame;
    CardModel mCardsModel;
    QQuickItem *mQmlObject;

public:
    CommonCardZone(Player *player, Game *game, std::string_view name);

    QQuickItem* visualItem() const override { return mQmlObject; }
    std::vector<Card> & cards() override { return mCardsModel.cards(); }
    CardModel& model() override { return mCardsModel; }
    void removeCard(int index) override { mCardsModel.removeCard(index); }
};
