#pragma once

#include <QObject>
#include <QList>
#include <QAbstractListModel>

class QQuickItem;
class QQmlContext;
class Player;
class Game;

struct HandCard
{
    QString code;
    bool glow;
};

class HandModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<HandCard> mCards;

public:
    enum HandCardRoles {
        CodeRole = Qt::UserRole + 1,
        GlowRole
    };

    HandModel(QObject *parent = 0) : QAbstractListModel(parent) {}

    void addCard(HandCard &&card);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
};


class Hand
{
    Player *mPlayer;
    Game *mGame;
    HandModel mCardsModel;
    QQuickItem *mQmlHand;
    QQmlContext *mContext;
public:
    Hand(Player *player, Game *game);

    QQuickItem* visualItem() const { return mQmlHand; }
    void addCard(HandCard &&card);
    void setHand(QList<QObject*> &handModel);
    void clearHand();
    void startMulligan();
    void endMulligan();
};
