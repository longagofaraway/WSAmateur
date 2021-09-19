#include "baseComponent.h"

#include <QQmlContext>

BaseComponent::BaseComponent(const QString &moduleName, QQuickItem *parent) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + moduleName + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);

    connect(qmlObject, SIGNAL(cancel()), this, SIGNAL(close()));
    connect(qmlObject, SIGNAL(componentReady()), this, SLOT(componentReady()));
}

BaseComponent::~BaseComponent() {
    qmlObject->deleteLater();
}
