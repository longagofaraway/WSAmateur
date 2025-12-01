#include "baseComponent.h"

#include <QQmlContext>

BaseComponent::BaseComponent(const QString &moduleName, QQuickItem *parent, QString componentId)
    : componentId_(componentId) {
    init(moduleName, parent, componentId);
}

BaseComponent::BaseComponent(const QString &moduleName, QQuickItem *parent) {
    init(moduleName, parent, "");
}

void BaseComponent::init(const QString &moduleName, QQuickItem *parent, QString componentId) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + moduleName + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    context->setContextProperty("componentId", componentId);
    QObject *obj = component.create(context);
    qmlObject_ = qobject_cast<QQuickItem*>(obj);
    qmlObject_->setParentItem(parent);
    qmlObject_->setParent(parent);
}

BaseComponent::~BaseComponent() {
    qmlObject_->deleteLater();
}
