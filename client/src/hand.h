#pragma once

#include <QObject>
#include <QList>

class QQuickItem;
class Player;
class Game;

class HandCard : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString code READ code NOTIFY codeChanged)
    Q_PROPERTY(bool glow READ glow NOTIFY glowChanged)
public:
    HandCard (QString code, bool glow)
        : mCode(code), mGlow(glow) {}
    QString code() { return mCode; }
    bool glow() { return mGlow; }

signals:
    void codeChanged();
    void glowChanged();

private:
    QString mCode;
    bool mGlow;
};

class Hand {
    Player *mPlayer;
    Game *mGame;
    QList<QObject*> mCardsModel;
    QQuickItem *mQmlHand;
public:
    Hand(Player *player, Game *game);
};
