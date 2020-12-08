#pragma once

#include <QObject>
#include <QList>

class QQuickItem;
class QQmlContext;
class Player;
class Game;

class HandCard : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString code READ code NOTIFY codeChanged)
    Q_PROPERTY(bool glow READ glow WRITE setGlow NOTIFY glowChanged)
public:
    HandCard (QString code, bool glow)
        : mCode(code), mGlow(glow) {}
    QString code() { return mCode; }
    bool glow() { return mGlow; }

    void setGlow(bool glow) {
        mGlow = glow;
        emit glowChanged();
    }

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
    QQmlContext *mContext;
public:
    Hand(Player *player, Game *game);

    QQuickItem* visualItem() const { return mQmlHand; }
    void setHand(QList<QObject*> &handModel);
    void clearHand();
};
