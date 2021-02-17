#pragma once

#include <QQuickItem>

#include "cardZone.h"

class Player;
class Game;

class DeckView : public CardZone
{
    Player *mPlayer;
    QQuickItem *mQmlObject;

public:
    DeckView(Player *player, Game *game);

    QQuickItem* visualItem() const override { return mQmlObject; }

    template<typename T>
    void setCards(const T &arr) {
        mCardsModel.clear();
        for (const auto &str: arr)
            mCardsModel.addCard(str);
        mQmlObject->setProperty("visible", true);
    }

    void hide() {
        mQmlObject->setProperty("visible", false);
    }
};
