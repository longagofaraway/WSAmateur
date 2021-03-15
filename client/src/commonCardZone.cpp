#include "commonCardZone.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

namespace {
QString qmlModuleFromZoneName(std::string_view v) {
    if (v == "wr")
        return "WaitingRoom";
    else if (v == "deck")
        return "Deck";
    else if (v == "clock")
        return "Clock";
    else if (v == "stock")
        return "Stock";
    else if (v == "level")
        return "Level";
    else if (v == "climax")
        return "Climax";
    else if (v == "res")
        return "ResolutionZone";
    else if (v == "view")
        return "OrderedCardsView";
    assert(false);
    return "";
}
}

CommonCardZone::CommonCardZone(Player *player, Game *game, std::string_view name)
    : mPlayer(player), mGame(game) {
    mName = name;
    QQmlComponent component(mGame->engine(), "qrc:/qml/" + qmlModuleFromZoneName(name) + ".qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(mGame);
    mQmlObject->setParent(mGame);
    mQmlObject->setProperty("opponent", player->isOpponent());
}
