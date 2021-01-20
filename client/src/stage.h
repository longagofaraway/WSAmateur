#pragma once

#include "cardModel.h"
#include "cardZone.h"

class Player;
class Game;

class Stage: public CardZone
{
    Player *mPlayer;
    Game *mGame;
    CardModel mCardsModel;
    QQuickItem *mQmlObject;

public:
    Stage(Player *player, Game *game);

    QQuickItem* visualItem() const override { return mQmlObject; }
    std::vector<Card> & cards() override { return mCardsModel.cards(); }
    CardModel& model() { return mCardsModel; }
    void removeCard(int index) override { mCardsModel.removeCard(index); }

    void mainPhase();
    void endMainPhase();
    void attackDeclarationStep();
    void highlightAttackers(bool highlight);
    void unhighlightAttacker();
    void attackDeclared(int pos);
    void endAttackPhase();
    void encoreStep();
    void deactivateEncoreStep();
};
