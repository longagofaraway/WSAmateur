#include "componentManager.h"

#include <QDebug>
#include <QQmlContext>

#include "ability_maker_gen.h"
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
    auto creator = std::make_shared<gen::ComponentMediator>(linkObject);
    if (name == "Target") {
        CppComponentPack pack{.component = std::make_unique<TargetComponent>(parent, id, displayName), .creator = creator};
        cppComponents_[id] = std::move(pack);
        auto *component = cppComponents_[id].component.get();
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setTarget, component, [=](const asn::Target& target, QString idParam) {
                auto *component = dynamic_cast<TargetComponent*>(cppComponents_[idParam].component.get());
                component->setTarget(target);
            });
        }
        connections_.insert(name);
        QObject::connect(component, &BaseComponent::targetReady, cppComponents_[id].creator.get(), &gen::ComponentMediator::targetChanged);
        return component->getQmlObject();
    }
    if (name == "CardSpecifier") {
        CppComponentPack pack{.component = std::make_unique<CardSpecifierComponent>(parent, id), .creator = creator};
        cppComponents_[id] = std::move(pack);
        auto *component = cppComponents_[id].component.get();
        QObject::connect(component, SIGNAL(deleteComponent(QString)), linkObject, SLOT(deleteCardSpecifier(QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setCardSpecifier, this, [=](const asn::CardSpecifier& cardSpecifier, QString idParam) {
                auto *component = dynamic_cast<CardSpecifierComponent*>(cppComponents_[idParam].component.get());
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
    QmlComponentPack pack{.component = object, .creator = std::make_shared<gen::ComponentMediator>(linkObject)};
    components_[id] = std::move(pack);
    auto qmlValueSetter = [=](QString value, QString idParam) {
        QMetaObject::invokeMethod(components_[idParam].component, "setValue", Q_ARG(QVariant, value));
    };
    auto qmlBoolValueSetter = [=](bool value, QString idParam) {
        QMetaObject::invokeMethod(components_[idParam].component, "setValue", Q_ARG(QVariant, value));
    };
    if (name == "Zone") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(zoneChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setZone, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "Phase") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(phaseChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setPhase, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "PhaseState") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(phaseStateChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setPhaseState, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "Player") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(playerChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setPlayer, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "State") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(stateChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setState, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "AttackType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(attackTypeChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setAttackType, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "AbilityType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(abilityTypeChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setAbilityType, this, qmlValueSetter);
        }
        connections_.insert(name);
    } else if (name == "Bool") {
        QObject::connect(object, SIGNAL(valueChanged(bool,QString)), components_[id].creator.get(), SLOT(boolChanged(bool,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setBool, this, qmlBoolValueSetter);
        }
        connections_.insert(name);
    } else if (name == "String") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), components_[id].creator.get(), SLOT(stringChanged(QString,QString)));
        if (!connections_.contains(name)) {
            QObject::connect(linkObject, &BaseComponent::setBool, this, qmlBoolValueSetter);
        }
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
        component.component->deleteLater();
    }
    components_.clear();
    cppComponents_.clear();
    connections_.clear();
}

void ComponentManager::deleteComponent(QString id) {
    if (cppComponents_.contains(id)) {
        cppComponents_.erase(id);
    } else if (components_.contains(id)) {
        components_[id].component->deleteLater();
        components_.remove(id);
    }
}
