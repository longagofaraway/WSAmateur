#pragma once

#include "cardModel.h"
#include "cardZone.h"

class Player;
class Game;

class CommonCardZone: public CardZone
{
    Player *mPlayer;
    Game *mGame;
    QQuickItem *mQmlObject;

public:
    CommonCardZone(Player *player, Game *game, std::string_view name);

    QQuickItem* visualItem() const override { return mQmlObject; }
};
