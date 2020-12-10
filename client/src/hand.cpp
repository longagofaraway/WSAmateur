#include "hand.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"


void HandModel::addCard(HandCard &&card) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mCards.emplace_back(std::move(card));
    endInsertRows();
}

bool HandModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.row() < 0 || static_cast<size_t>(index.row()) >= mCards.size())
        return false;

    if (role != GlowRole)
        return false;

    HandCard &card = mCards[index.row()];

    card.glow = value.toBool();
    emit dataChanged(index, index, { GlowRole });
    return true;
}

int HandModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(mCards.size());
}

QVariant HandModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || static_cast<size_t>(index.row()) >= mCards.size())
        return QVariant();

    const HandCard &card = mCards[index.row()];
    switch(role) {
    case CodeRole:
        return card.code;
    case GlowRole:
        return card.glow;
    default:
        return QVariant();
    }
}


QHash<int, QByteArray> HandModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[CodeRole] = "code";
        (*roles)[GlowRole] = "glow";
    }
    return *roles;
}


Hand::Hand(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    /*mCardsModel = {
        new HandCard("IMC/W43-111", false),
        new HandCard("IMC/W43-009", false),
        new HandCard("IMC/W43-046", true),
        new HandCard("IMC/W43-127", false),
        new HandCard("IMC/W43-091", false),
        new HandCard("IMC/W43-046", false),
        new HandCard("IMC/W43-009", false)
    };*/

    QQmlComponent component(mGame->engine(), "qrc:/qml/Hand.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame->parent());
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlHand = qobject_cast<QQuickItem*>(obj);
    mQmlHand->setParentItem(mGame->parentItem());
    mQmlHand->setParent(mGame->parent());
    mQmlHand->setProperty("opponent", player->isOpponent());
}

void Hand::addCard(HandCard &&card) {
    mCardsModel.addCard(std::move(card));
}

void Hand::setHand(QList<QObject *> &handModel) {
    //clearHand();
    //mCardsModel = handModel;

    //mContext->setContextProperty("innerModel", QVariant::fromValue(mCardsModel));
}

void Hand::clearHand() {
    /*while (mCardsModel.size()) {
        delete mCardsModel.takeLast();
    }*/
}

void Hand::startMulligan() {
    mQmlHand->setState("mulligan");
    mQmlHand->connect(mQmlHand, SIGNAL(cardSelected(bool)), mGame, SLOT(cardSelectedForMulligan(bool)));
}

void Hand::endMulligan() {
    mQmlHand->setState("");
    mQmlHand->disconnect(mQmlHand, SIGNAL(cardSelected(bool)), mGame, SLOT(cardSelectedForMulligan(bool)));
}
