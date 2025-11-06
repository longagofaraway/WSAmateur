#include "componentManager.h"

#include <QQmlContext>

#include "componentHelper.h"
#include "cardSpecifier.h"
#include "target.h"

namespace {
const QSet<QString> kCppComponents {
    "Target",
    "CardSpecifier"
};

QQuickItem* createQmlObject(const QString &name, const QString &id, const QString &displayName, QQuickItem *parent) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    context->setContextProperty("componentId", id);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setProperty("displayName", displayName);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    return qmlObject;
}
}

QQuickItem* ComponentManager::createComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject) {
    if (kCppComponents.contains(name)) {
        return createCppComponent(name, displayName, id, parent, linkObject);
    }
    return createQmlComponent(name, displayName, id, parent, linkObject);
}

QQuickItem* ComponentManager::createCppComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject) {
    if (name == "Target") {
        cppComponents_[id.toStdString()] = std::make_unique<TargetComponent>(parent, id, displayName);
        auto *component = cppComponents_[id.toStdString()].get();
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setTarget, component, [=](const asn::Target& target, QString idParam) {
                auto *component = dynamic_cast<TargetComponent*>(cppComponents_[idParam.toStdString()].get());
                component->setTarget(target);
            });
        }
        connections_.insert(name);
        QObject::connect(component, SIGNAL(targetReady(asn::Target)), linkObject, SLOT(targetChanged(asn::Target)));
        return component->getQmlObject();
    }
    if (name == "CardSpecifier") {
        cppComponents_[id.toStdString()] = std::make_unique<CardSpecifierComponent>(parent, id);
        auto *component = cppComponents_[id.toStdString()].get();
        QObject::connect(component, SIGNAL(deleteComponent(QString)), linkObject, SLOT(deleteCardSpecifier(QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setCardSpecifier, this, [=](const asn::CardSpecifier& cardSpecifier, QString idParam) {
                auto *component = dynamic_cast<CardSpecifierComponent*>(cppComponents_[idParam.toStdString()].get());
                component->setCardSpecifier(cardSpecifier);
            });
        }
        connections_.insert(name);
        QObject::connect(component, SIGNAL(componentReady(asn::CardSpecifier,QString)), linkObject, SLOT(cardSpecifierChanged(asn::CardSpecifier,QString)));
        return component->getQmlObject();
    }
    return nullptr;
}

QQuickItem* ComponentManager::createQmlComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject) {
    QQuickItem *object = createQmlObject(getBasicComponentQmlPath(name), id, displayName, parent);
    components_[id] = object;
    auto qmlValueSetter = [=](QString value, QString idParam) {
        QMetaObject::invokeMethod(components_[idParam], "setValue", Q_ARG(QVariant, value));
    };
    auto qmlBoolValueSetter = [=](bool value, QString idParam) {
        QMetaObject::invokeMethod(components_[idParam], "setValue", Q_ARG(QVariant, value));
    };
    if (name == "Zone") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(zoneChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setZone, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "Phase") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(phaseChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setPhase, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "PhaseState") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(phaseStateChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setPhaseState, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "Player") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(playerChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setPlayer, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "State") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(stateChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setState, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "AttackType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(attackTypeChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setAttackType, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "AbilityType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(abilityTypeChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setAbilityType, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "Bool") {
        QObject::connect(object, SIGNAL(valueChanged(bool,QString)), linkObject, SLOT(boolChanged(bool,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setBool, this, qmlBoolValueSetter);
        }
        connections_.insert(name);
    }
    return object;
}

void ComponentManager::clear() {
    for (auto *component: qAsConst(components_)) {
        component->deleteLater();
    }
    components_.clear();
    cppComponents_.clear();
}

void ComponentManager::deleteComponent(QString id) {
    if (cppComponents_.contains(id.toStdString())) {
        cppComponents_.erase(id.toStdString());
    } else if (components_.contains(id)) {
        components_[id]->deleteLater();
        components_.remove(id);
    }
}
