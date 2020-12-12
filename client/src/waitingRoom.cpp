#include "waitingRoom.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

WaitingRoom::WaitingRoom(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    QQmlComponent component(mGame->engine(), "qrc:/qml/WaitingRoom.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame->parent());
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(mGame->parentItem());
    mQmlObject->setParent(mGame->parent());
    mQmlObject->setProperty("opponent", player->isOpponent());
}
