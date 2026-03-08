#pragma once

#include <QQuickItem>

#include "baseComponent.h"
#include "languageSpecification.h"

class ComponentManager;

class ArrayComponent : public BaseComponent {
    Q_OBJECT
private:
    size_t arraySize_{1};
    LangComponent langComponent_;
    BaseComponent* linkObject_;
    ComponentManager* componentManager_;
    QQuickItem *parent_;
    QObject *mediator_;
    int typeIndex_;

public:
    ArrayComponent(const LangComponent& langComponent, QString componentId, int typeIndex, QQuickItem *parent, BaseComponent* linkObject, ComponentManager *componentManager, QObject *mediator);

public slots:
    void addComponent();
};
