#include "hand.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

Hand::Hand(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    mCardsModel = {
        new HandCard("qrc:///resources/images/imc.jpg", "q1", false),
        new HandCard("qrc:///resources/images/imc3.jpg", "q2", false),
        new HandCard("qrc:///resources/images/imc4.jpg", "q3", true),
        new HandCard("qrc:///resources/images/imc2.jpg", "q4", false)
    };

    QQmlComponent component(mGame->engine(), "qrc:/qml/Hand.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame->parent());
    context->setContextProperty("innerModel", QVariant::fromValue(mCardsModel));
    QObject *obj = component.create(context);
    mQmlHand = qobject_cast<QQuickItem*>(obj);
    mQmlHand->setParentItem(mGame->parentItem());
    mQmlHand->setParent(mGame->parent());
    mQmlHand->setProperty("opponent", player->isOpponent());
}
