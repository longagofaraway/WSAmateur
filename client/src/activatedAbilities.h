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

    void activatePlay(int index, bool active);
    void activateCancel(int index, bool active);
    void setActiveByUniqueId(uint32_t uniqueId, bool active);
    ActivatedAbility& ability(int index) { return mModel.ability(index); }
    int activeId() const { return mModel.activeId(); }
    void removeActiveAbility();
    void setActive(int index, bool active) { mModel.setActive(index, active); }
    void addAbility(const ActivatedAbility &a) { mModel.addAbility(a); }
    int count() const { return mModel.count(); }
};

