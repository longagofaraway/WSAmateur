#pragma once

#include "cardModel.h"
#include "cardZone.h"

class Player;
class Game;

class Stage: public CardZone
{
    Game *mGame;
    QQuickItem *mQmlObject;

public:
    Stage(Player *player, Game *game);

    QQuickItem* visualItem() const override { return mQmlObject; }

    void setAttr(int row, ProtoCardAttribute attr, int value);
    void mainPhase();
    void endMainPhase();
    void attackDeclarationStep();
    void highlightAttackers(bool highlight);
    void unhighlightAttacker();
    void attackDeclared(int pos);
    void endAttackPhase();
    void encoreStep();
    void deactivateEncoreStep();
    void swapCards(int from, int to);
};
