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

    const std::vector<Card>& cards() const override;
    QQuickItem* visualItem() const override { return mQmlHand; }
    void addCard(Card &&card);
    void removeCard(int index) { mCardsModel.removeCard(index); }
    void startMulligan();
    void endMulligan();
};
