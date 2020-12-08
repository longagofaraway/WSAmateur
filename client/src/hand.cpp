#include "hand.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

Hand::Hand(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    mCardsModel = {
        new HandCard("IMC/W43-111", false),
        new HandCard("IMC/W43-009", false),
        new HandCard("IMC/W43-046", true),
        new HandCard("IMC/W43-127", false),
        new HandCard("IMC/W43-091", false),
        new HandCard("IMC/W43-046", false),
        new HandCard("IMC/W43-009", false)
    };

    QQmlComponent component(mGame->engine(), "qrc:/qml/Hand.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame->parent());
    mContext = context;
    context->setContextProperty("innerModel", QVariant::fromValue(mCardsModel));
    QObject *obj = component.create(context);
    mQmlHand = qobject_cast<QQuickItem*>(obj);
    mQmlHand->setParentItem(mGame->parentItem());
    mQmlHand->setParent(mGame->parent());
    mQmlHand->setProperty("opponent", player->isOpponent());
    /*if (!player->isOpponent()) {
        mQmlHand->setProperty("state", "mulligan");
        mQmlHand->setProperty("mulligan", true);
    }*/
}

void Hand::setHand(QList<QObject *> &handModel) {
    //clearHand();
    mCardsModel = handModel;

    //mContext->setContextProperty("innerModel", QVariant::fromValue(mCardsModel));
}

void Hand::clearHand() {
    while (mCardsModel.size()) {
        delete mCardsModel.takeLast();
    }
}
