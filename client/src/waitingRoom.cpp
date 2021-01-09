#include "waitingRoom.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

WaitingRoom::WaitingRoom(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    QQmlComponent component(mGame->engine(), "qrc:/qml/WaitingRoom.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(mGame);
    mQmlObject->setParent(mGame);
    mQmlObject->setProperty("opponent", player->isOpponent());
}
