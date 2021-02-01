#include "activatedAbilities.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

ActivatedAbilities::ActivatedAbilities(Player *player, Game *game) {
    QQmlComponent component(game->engine(), "qrc:/qml/ActivatedAbilityList.qml");
    QQmlContext *context = new QQmlContext(game->context(), game);
    context->setContextProperty("innerModel", QVariant::fromValue(&mModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(game);
    mQmlObject->setParent(game);
    mQmlObject->setProperty("mOpponent", player->isOpponent());
}
