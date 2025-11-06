#include "abilityMaker.h"

#include <QQmlContext>

#include "trigger.h"
#include "triggerInit.h"

namespace {
QQuickItem* createQmlObject(const QString &name, QQuickItem *parent) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    return qmlObject;
}
}

void AbilityMaker::componentComplete() {
    QQuickItem::componentComplete();
    abilityMenu = createQmlObject("AbilityMenu", this);
    connect(abilityMenu, SIGNAL(createTrigger(QString)), this, SLOT(createTrigger(QString)));
    qvariant_cast<QObject*>(abilityMenu->property("anchors"))->setProperty("bottom", this->property("bottom"));
    qvariant_cast<QObject*>(abilityMenu->property("anchors"))->setProperty("top", this->property("top"));
    qvariant_cast<QObject*>(abilityMenu->property("anchors"))->setProperty("topMargin", 140);
    workingArea = createQmlObject("WorkingArea", this);
    qvariant_cast<QObject*>(workingArea->property("anchors"))->setProperty("left", abilityMenu->property("right"));
    qvariant_cast<QObject*>(workingArea->property("anchors"))->setProperty("top", abilityMenu->property("top"));
    qvariant_cast<QObject*>(workingArea->property("anchors"))->setProperty("right", this->property("right"));
    qvariant_cast<QObject*>(workingArea->property("anchors"))->setProperty("bottom", this->property("bottom"));
    QMetaObject::invokeMethod(abilityMenu, "setWorkingArea", Q_ARG(QVariant, QVariant::fromValue(workingArea)));
}

void AbilityMaker::createTrigger(QString triggerId) {
    auto trigger = getTriggerFromPreset(triggerId);
    if (trigger.has_value()) {
        qmlObject = std::make_shared<TriggerComponent>(workingArea, this, trigger.value());
    } else {
        qmlObject = std::make_shared<TriggerComponent>(workingArea, this);
    }

}
