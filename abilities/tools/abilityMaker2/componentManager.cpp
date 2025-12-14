#include "componentManager.h"

#include <QDebug>
#include <QQmlContext>

#include "ability_maker_gen.h"
#include "componentHelper.h"
#include "card.h"
#include "cardSpecifier.h"
#include "target.h"

namespace {
const QSet<QString> kCppComponents {
    "Target",
    "CardSpecifier",
    "Card"
};

QQuickItem* createQmlObject(const QString &name, const QString &id, const QString &displayName, QQuickItem *parent, BaseComponent *linkObject) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    context->setContextProperty("componentId", id);
    context->setContextProperty("parentComponent", linkObject);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setProperty("displayName", displayName);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    return qmlObject;
}
}

QQuickItem* ComponentManager::createComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject, gen::ComponentMediator *mediator) {
    if (kCppComponents.contains(name)) {
        return createCppComponent(name, displayName, id, parent, linkObject, mediator);
    }
    return createQmlComponent(name, displayName, id, parent, linkObject, mediator);
}

QQuickItem* ComponentManager::createCppComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject, gen::ComponentMediator *mediator) {
    if (name == "Target") {
        cppComponents_[id] = std::make_unique<TargetComponent>(parent, id, displayName);
        auto *component = cppComponents_[id].get();
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setTarget, component, [=](const asn::Target& target, QString idParam) {
                auto *component = dynamic_cast<TargetComponent*>(cppComponents_[idParam].get());
                component->setTarget(target);
            });
        }
        connections_.insert(name);
        QObject::connect(component, SIGNAL(targetReady(asn::Target,QString)), mediator, SLOT(targetChanged(asn::Target,QString)));
        return component->getQmlObject();
    }
    if (name == "CardSpecifier") {
        cppComponents_[id] = std::make_unique<CardSpecifierComponent>(parent, id);
        auto *component = cppComponents_[id].get();
        QObject::connect(component, SIGNAL(deleteComponent(QString)), linkObject, SLOT(deleteCardSpecifier(QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setCardSpecifier, this, [=](const asn::CardSpecifier& cardSpecifier, QString idParam) {
                auto *component = dynamic_cast<CardSpecifierComponent*>(cppComponents_[idParam].get());
                component->setCardSpecifier(cardSpecifier);
            });
        }
        connections_.insert(name);
        QObject::connect(component, SIGNAL(componentReady(asn::CardSpecifier,QString)), linkObject, SLOT(cardSpecifierChanged(asn::CardSpecifier,QString)));
        return component->getQmlObject();
    }
    if (name == "Card") {
        cppComponents_[id] = std::make_unique<CardComponent>(name, parent, id, name);
        auto *component = cppComponents_[id].get();
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setCard, this, [=](const asn::Card& card, QString idParam) {
                auto *component = dynamic_cast<CardComponent*>(cppComponents_[idParam].get());
                component->setCard(card);
            });
        }
        connections_.insert(name);
        QObject::connect(component, SIGNAL(cardReady(asn::Card,QString)), mediator, SLOT(cardChanged(asn::Card,QString)));
        return component->getQmlObject();
    }
    return nullptr;
}

QQuickItem* ComponentManager::createQmlComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent *linkObject, gen::ComponentMediator *mediator) {
    QQuickItem *object = createQmlObject(getBasicComponentQmlPath(name), id, displayName, parent, linkObject);
    components_[id] = object;
    if (name == "Zone") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(zoneChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "Phase") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(phaseChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "PhaseState") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(phaseStateChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "Player") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(playerChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "State") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(stateChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "AttackType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(attackTypeChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "AbilityType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(abilityTypeChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "AttributeType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(attributeTypeChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "ValueType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(valueTypeChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "Bool") {
        QObject::connect(object, SIGNAL(valueChanged(bool,QString)), mediator, SLOT(boolChanged(bool,QString)));
        connections_.insert(name);
    } else if (name == "String") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(stringChanged(QString,QString)));
        connections_.insert(name);
    } else if (name == "Int32") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(int32Changed(QString,QString)));
        connections_.insert(name);
    } else if (name == "Int8") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(int8Changed(QString,QString)));
        connections_.insert(name);
    } else if (name == "UInt8") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(uInt8Changed(QString,QString)));
        connections_.insert(name);
    } else if (name == "Duration") {
        QObject::connect(object, SIGNAL(valueChanged(int,QString)), mediator, SLOT(durationChanged(int,QString)));
        connections_.insert(name);
    } else {
        qWarning() << "did not find component to create";
    }
    return object;
}


ComponentManager::~ComponentManager() {
    clear();
}

void ComponentManager::clear() {
    for (auto &component: qAsConst(components_)) {
        component->deleteLater();
    }
    components_.clear();
    cppComponents_.clear();
    connections_.clear();
}

void ComponentManager::deleteComponent(QString id) {
    if (cppComponents_.contains(id)) {
        cppComponents_.erase(id);
    } else if (components_.contains(id)) {
        components_[id]->deleteLater();
        components_.remove(id);
    }
}
