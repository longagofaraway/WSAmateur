#pragma once

#include "cardModel.h"
#include "cardZone.h"

class Player;
class Game;

class WaitingRoom: public CardZone
{
    Player *mPlayer;
    Game *mGame;
    CardModel mCardsModel;
    QQuickItem *mQmlObject;

public:
    WaitingRoom(Player *player, Game *game);

    QQuickItem* visualItem() const override { return mQmlObject; }
    const std::vector<Card> & cards() const override { return mCardsModel.cards(); }
    void removeCard(int index) override { mCardsModel.removeCard(index); }
};
