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

void ActivatedAbilities::activatePlay(int index, bool active) {
    mModel.activatePlayButton(index, active);
}

void ActivatedAbilities::activateCancel(int index, bool active) {
    mModel.activateCancelButton(index, active);
}

void ActivatedAbilities::setActiveByUniqueId(uint32_t uniqueId, bool active) {
    mModel.setActive(mModel.idByUniqueId(uniqueId), active);
}

void ActivatedAbilities::removeActiveAbility() {
    int id = mModel.activeId();
    mModel.setActive(id, false);
    mModel.removeAbility(id);
}
