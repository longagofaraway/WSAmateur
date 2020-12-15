#include "commonCardZone.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

#include <QDebug>

CommonCardZone::CommonCardZone(Player *player, Game *game, std::string_view name)
    : mPlayer(player), mGame(game) {
    qDebug() << "qrc:/qml/" + QString::fromStdString(std::string(name)) + ".qml";
    QQmlComponent component(mGame->engine(), "qrc:/qml/" + QString::fromStdString(std::string(name)) + ".qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame->parent());
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(mGame->parentItem());
    mQmlObject->setParent(mGame->parent());
    //mQmlObject->setProperty("opponent", player->isOpponent());
}
