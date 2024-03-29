#include "activatedAbilities.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

ActivatedAbilities::ActivatedAbilities(Player *player, Game *game) {
    mModel.init(player);
    QQmlComponent component(game->engine(), "qrc:/qml/ActivatedAbilityList.qml");
    QQmlContext *context = new QQmlContext(game->context(), game);
    context->setContextProperty("innerModel", QVariant::fromValue(&mModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(game);
    mQmlObject->setParent(game);
    mQmlObject->setProperty("mOpponent", player->isOpponent());
}

void ActivatedAbilities::activatePlay(int index, bool active, const char *text) {
    mModel.activatePlayButton(index, active);
    mModel.setPlayBtnText(index, text);
}

void ActivatedAbilities::activateCancel(int index, bool active) {
    mModel.activateCancelButton(index, active);
}

void ActivatedAbilities::setActiveByUniqueId(uint32_t uniqueId, bool active) {
    mModel.setActive(mModel.indexByUniqueId(uniqueId), active);
}

void ActivatedAbilities::removeActiveAbility() {
    int id = mModel.activeId();
    mModel.setActive(id, false);
    mModel.removeAbility(id);
}

void ActivatedAbilities::clear() {
    for (int i = 0; i < mModel.count(); ++i)
        mModel.removeAbility(i);
}
