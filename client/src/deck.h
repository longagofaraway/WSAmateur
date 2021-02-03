#pragma once

#include "cardModel.h"
#include "cardZone.h"

class Player;
class Game;

class Deck: public CardZone
{
    Player *mPlayer;
    Game *mGame;
    QQuickItem *mQmlObject;

public:
    Deck(Player *player, Game *game);

    QQuickItem* visualItem() const override { return mQmlObject; }
};
