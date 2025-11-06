#include "baseComponent.h"

#include <QQmlContext>

BaseComponent::BaseComponent(const QString &moduleName, QQuickItem *parent, QString componentId) {
    init(moduleName, parent, componentId);

    //connect(qmlObject, SIGNAL(componentReady()), this, SLOT(componentReady()));
}

BaseComponent::BaseComponent(const QString &moduleName, QQuickItem *parent) {
    init(moduleName, parent, "");
}

void BaseComponent::init(const QString &moduleName, QQuickItem *parent, QString componentId) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + moduleName + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    context->setContextProperty("componentId", componentId);
    QObject *obj = component.create(context);
    qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
}

BaseComponent::~BaseComponent() {
    qmlObject->deleteLater();
}
