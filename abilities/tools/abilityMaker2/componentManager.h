#pragma once

#include <memory>
#include <vector>

#include <QQuickItem>

#include "baseComponent.h"

namespace gen {
    class ComponentMediator;
}
struct LangComponent;
class ArrayComponent;

class ComponentManager : public QObject {
    Q_OBJECT

    using MemberFunctionPtr = QQuickItem* (ComponentManager::*)(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    std::unordered_map<QString, std::unique_ptr<BaseComponent>> cppComponents_;
    QHash<QString, QQuickItem*> components_;
    QHash<QString, QMetaObject::Connection> connections_;
    std::unordered_map<QString, std::unique_ptr<ArrayComponent>> arrayComponents_;
    std::unordered_map<QString, MemberFunctionPtr> creatorsMap_;

public:
    ComponentManager();
    ~ComponentManager();
    std::vector<QQuickItem*> createComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, gen::ComponentMediator *mediator, int typeIndex, size_t arraySize);
    QQuickItem* addComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator, int typeIndex, int arrayPosition);
    std::vector<QQuickItem*> getComponentsRow(QString type, int typeIndex);
    void deleteComponent(QString id);
    void clear();

    static QString buildComponentId(QString type, int typeIndex, int arrayPosition);

private:
    std::vector<QQuickItem*> createCppComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator, int typeIndex, size_t arraySize);
    std::vector<QQuickItem*> createQmlComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator, int typeIndex, size_t arraySize);
    QQuickItem* createQmlComponentInternal(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent *linkObject, QObject *mediator);

    QQuickItem* createTarget(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createCardSpecifier(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createCard(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createTargetAndPlace(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createPlace(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createNumber(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createMultiplier(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createCondition(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createAbility(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createAutoAbility(QString id, const LangComponent &langComponent, QQuickItem *parent, BaseComponent *linkObject, QObject *mediator);
    QQuickItem* createEventAbility(QString id, const LangComponent &langComponent, QQuickItem *parent, BaseComponent *linkObject, QObject *mediator);
};
