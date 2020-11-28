#pragma once

#include <QObject>
#include <QList>

class QQuickItem;
class Player;
class Game;

class HandCard : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString src READ src NOTIFY srcChanged)
    Q_PROPERTY(QString code READ code NOTIFY codeChanged)
    Q_PROPERTY(bool glow READ glow NOTIFY glowChanged)
public:
    HandCard (QString src, QString code, bool glow)
        : mSrc(src), mCode(code), mGlow(glow) {}
    QString src() { return mSrc; }
    QString code() { return mCode; }
    bool glow() { return mGlow; }

signals:
    void srcChanged();
    void codeChanged();
    void glowChanged();

private:
    QString mSrc;
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
