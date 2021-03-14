#include "deckView.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

DeckView::DeckView(Player *player, Game *game)
    : mPlayer(player) {
    mName = "deckView";
    QQmlComponent component(game->engine(), "qrc:/qml/CardsView.qml");
    QQmlContext *context = new QQmlContext(game->context(), game);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(game);
    mQmlObject->setParent(game);
    mQmlObject->setProperty("mOpponent", player->isOpponent());
    mQmlObject->setProperty("mIsDeckView", true);
}
