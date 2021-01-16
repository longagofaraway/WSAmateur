#include "abilityMaker.h"

#include <QUuid>

#include "abilities.h"

namespace {
QString generateName() {
    return QUuid::createUuid().toString();
}

AutoAbility createAutoAbility(QObject *item) {
    AutoAbility a;
    a.activationTimes = item->property("activationTimes").toInt();
    auto llist = item->property("keywords").toList();
    int w = llist[0].toInt();

    return a;
}
}

void AbilityMaker::createAbility() {
    auto type = this->property("abilityType").toUInt();
    auto abilityId = this->property("ability").toString();
    QObject *item = this->findChild<QObject*>(abilityId);

    Ability ability;
    ability.type = static_cast<AbilityType>(type);
    switch (type) {
    case 2:
        ability.ability = createAutoAbility(item);
        break;
    }
}

QString AbilityMaker::createComponent(QString componentName) {
    QQmlComponent component(qmlEngine(this), "qrc:/" + componentName + ".qml");
    auto obj = component.create(qmlContext(this));
    QQuickItem *item = qobject_cast<QQuickItem*>(obj);
    item->setParentItem(this);
    item->setParent(this);
    auto newName = generateName();
    item->setProperty("objectName", newName);
    return newName;
}
