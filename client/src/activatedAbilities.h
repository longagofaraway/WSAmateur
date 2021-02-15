#pragma once

#include "activatedAbilityModel.h"

class Player;
class Game;

class QQuickItem;

class ActivatedAbilities
{
    ActivatedAbilityModel mModel;
    QQuickItem *mQmlObject;

public:
    ActivatedAbilities(Player *player, Game *game);

    void activatePlay(int index, bool active, const char *text = "Play");
    void activateCancel(int index, bool active);
    void setActiveByUniqueId(uint32_t uniqueId, bool active);
    ActivatedAbility& ability(int index) { return mModel.ability(index); }
    int activeId() const { return mModel.activeId(); }
    void removeActiveAbility();
    void setActive(int index, bool active) { mModel.setActive(index, active); }
    void addAbility(const ActivatedAbility &a) { mModel.addAbility(a); }
    int count() const { return mModel.count(); }
};

