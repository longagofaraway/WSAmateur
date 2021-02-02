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

    ActivatedAbility& activeAbility() { return mModel.activeAbility(); }
    void addAbility(const ActivatedAbility &a) { mModel.addAbility(a); }
    int count() const { return mModel.count(); }
};

