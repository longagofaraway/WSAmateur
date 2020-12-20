#include "stage.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

#include <QDebug>

Stage::Stage(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    mCardsModel.addCards(5);

    QQmlComponent component(mGame->engine(), "qrc:/qml/Stage.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(mGame);
    mQmlObject->setParent(mGame);
    mQmlObject->setProperty("opponent", player->isOpponent());
}

void Stage::mainPhase() {
    mQmlObject->connect(mQmlObject, SIGNAL(switchPositions(int, int)), mPlayer, SLOT(switchPositions(int, int)));
    mQmlObject->setProperty("mDragEnabled", true);
}
