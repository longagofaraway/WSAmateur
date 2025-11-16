#include "abilityMaker.h"

#include <QQmlContext>

#include "trigger.h"
#include "triggerInit.h"
#include "language_parser.h"

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
}

QString AbilityMaker::translate(const asn::Ability &ability) {
    //ability_ = ability;
    auto descr = QString::fromStdString(printAbility(ability));
    return descr;
    //QMetaObject::invokeMethod(this, "setDescription", Q_ARG(QVariant, descr));
}
