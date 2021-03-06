#include "commonCardZone.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

CommonCardZone::CommonCardZone(Player *player, Game *game, std::string_view name)
    : mPlayer(player), mGame(game) {
    QQmlComponent component(mGame->engine(), "qrc:/qml/" + QString::fromStdString(std::string(name)) + ".qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(mGame);
    mQmlObject->setParent(mGame);
    mQmlObject->setProperty("opponent", player->isOpponent());
}
