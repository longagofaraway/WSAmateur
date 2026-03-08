#include "array.h"

#include "componentManager.h"

ArrayComponent::ArrayComponent(const LangComponent &langComponent, QString componentId, int typeIndex, QQuickItem *parent, BaseComponent *linkObject, ComponentManager *componentManager, QObject *mediator)
    : BaseComponent("Array", parent, componentId), langComponent_(langComponent), linkObject_(linkObject), componentManager_(componentManager), parent_(parent), mediator_(mediator), typeIndex_(typeIndex) {
    connect(qmlObject_, SIGNAL(addComponent()), this, SLOT(addComponent()));
}

void ArrayComponent::addComponent() {
    componentManager_->addComponent(langComponent_, parent_, linkObject_, mediator_, typeIndex_, arraySize_);
    linkObject_->addComponentToArray(langComponent_.type, langComponent_.name, typeIndex_);
    arraySize_++;
}
