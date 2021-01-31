#pragma once

#include "abilityModel.h"

class Player;
class Game;

class QQuickItem;

class ActivatedAbilities
{
    AbilityModel mModel;
    QQuickItem *mQmlObject;
public:
    ActivatedAbilities(Player *player, Game *game);

    void addAbility(const ActivatedAbility &a) { mModel.addAbility(a); }
};

