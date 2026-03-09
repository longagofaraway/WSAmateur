#include "array.h"

#include "componentManager.h"

ArrayComponent::ArrayComponent(const LangComponent &langComponent, QString componentId, int typeIndex, QQuickItem *parent, BaseComponent *linkObject, ComponentManager *componentManager, QObject *mediator, size_t arraySize)
    : BaseComponent("Array", parent, componentId), arraySize_(arraySize), langComponent_(langComponent), linkObject_(linkObject), componentManager_(componentManager), parent_(parent), mediator_(mediator), typeIndex_(typeIndex) {
    qmlObject_->setProperty("currentNumber", arraySize_);
    connect(qmlObject_, SIGNAL(addComponent()), this, SLOT(addComponent()));
    connect(qmlObject_, SIGNAL(removeComponent()), this, SLOT(removeComponent()));
}

void ArrayComponent::addComponent() {
    componentManager_->addComponent(langComponent_, parent_, linkObject_, mediator_, typeIndex_, arraySize_);
    linkObject_->addComponentToArray(langComponent_.type, langComponent_.name, typeIndex_);
    arraySize_++;
}

void ArrayComponent::removeComponent() {
    arraySize_--;
    auto id = componentManager_->buildComponentId(langComponent_.type, typeIndex_, arraySize_);
    componentManager_->deleteComponent(id);
    linkObject_->removeComponentFromArray(langComponent_.type, langComponent_.name, typeIndex_);
}
