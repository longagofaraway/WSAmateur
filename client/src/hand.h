#pragma once

#include <QObject>
#include <QList>
#include <QAbstractListModel>

#include "cardModel.h"
#include "cardZone.h"

class QQuickItem;
class QQmlContext;
class Player;
class Game;

class Hand: public CardZone
{
    Game *mGame;
    QQuickItem *mQmlHand;

public:
    Hand(Player *player, Game *game);

    QQuickItem* visualItem() const override { return mQmlHand; }
    void addCard(const std::string &code);
    void addCard();
    void startMulligan();
    void endMulligan();
    void clockPhase();
    void endClockPhase();
    void playTiming();
    void endPlayTiming();
    void discardCard();
    void deactivateDiscarding();
};
