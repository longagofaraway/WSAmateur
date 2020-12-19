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
    Player *mPlayer;
    Game *mGame;
    CardModel mCardsModel;
    QQuickItem *mQmlHand;

public:
    Hand(Player *player, Game *game);

    std::vector<Card>& cards() override;
    QQuickItem* visualItem() const override { return mQmlHand; }
    CardModel& model() { return mCardsModel; }
    void addCard(const std::string &code);
    void addCard();
    void removeCard(int index) override { mCardsModel.removeCard(index); }
    void startMulligan();
    void endMulligan();
    void clockPhase();
    void endClockPhase();
};
