#include "componentManager.h"

#include <QDebug>
#include <QQmlContext>

#include "ability.h"
#include "ability_maker_gen.h"
#include "array.h"
#include "componentHelper.h"
#include "componentOpener.h"
#include "card.h"
#include "cardSpecifier.h"
#include "condition.h"
#include "languageSpecification.h"
#include "multiplier.h"
#include "number.h"
#include "place.h"
#include "target.h"
#include "targetAndPlace.h"

namespace {
const QSet<QString> kCppComponents {
    "Target",
    "TargetAndPlace",
    "CardSpecifier",
    "Card",
    "Place",
    "Number",
    "Multiplier",
    "Condition",
    "Ability"
};

QQuickItem* createQmlObject(const QString &name, const QString &id, const QString &displayName, QQuickItem *parent, BaseComponent *linkObject) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    context->setContextProperty("componentId", id);
    context->setContextProperty("parentComponent", linkObject);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setProperty("displayName", displayName);
    qmlObject->setProperty("componentName", displayName);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    return qmlObject;
}
}

std::vector<QQuickItem*> ComponentManager::createComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, gen::ComponentMediator *mediator, int typeIndex, size_t arraySize) {
    QObject *slotObject;
    if (mediator) slotObject = mediator;
    else slotObject = linkObject;
    if (kCppComponents.contains(langComponent.type)) {
        return createCppComponent(langComponent, parent, linkObject, slotObject, typeIndex, arraySize);
    }
    return {createQmlComponent(langComponent, parent, linkObject, slotObject, typeIndex, arraySize)};
}

QQuickItem* ComponentManager::addComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator, int typeIndex, int arrayPosition) {
    QObject *slotObject;
    if (mediator) slotObject = mediator;
    else slotObject = linkObject;
    QString id = buildComponentId(langComponent.type, typeIndex, arrayPosition);
    if (kCppComponents.contains(langComponent.type)) {
        return (this->*creatorsMap_[langComponent.type])(id, langComponent, parent, linkObject, slotObject);
    }
    return createQmlComponentInternal(id, langComponent, parent, linkObject, slotObject);
}

std::vector<QQuickItem*> ComponentManager::getComponentsRow(QString type, int typeIndex) {
    int i = 0;
    std::vector<QQuickItem*> result;
    while (true) {
        QString id = type + '/' + QString::number(typeIndex) + '/' + QString::number(i);
        bool found{false};
        if (cppComponents_.contains(id)) {
            result.push_back(cppComponents_[id]->getQmlObject());
            found = true;
        }
        if (components_.contains(id)) {
            result.push_back(components_[id]);
            found = true;
        }
        if (!found)
            break;
        i++;
    }
    QString arrayId = type + QString::number(typeIndex);
    if (arrayComponents_.contains(arrayId)) {
        result.push_back(arrayComponents_[arrayId]->getQmlObject());
    }

    return result;
}

QQuickItem* ComponentManager::createTarget(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    cppComponents_[id] = std::make_unique<TargetComponent>(parent, id, langComponent.name);
    auto *component = cppComponents_[id].get();
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setTarget, component,
            [=](const asn::Target& target, QString idParam) {
                auto *component = dynamic_cast<TargetComponent*>(cppComponents_[idParam].get());
                component->setTarget(target);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(component, SIGNAL(targetReady(asn::Target,QString)), mediator, SLOT(targetChanged(asn::Target,QString)));
    return component->getQmlObject();
}

QQuickItem* ComponentManager::createCardSpecifier(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    cppComponents_[id] = std::make_unique<CardSpecifierComponent>(parent, id);
    auto *component = cppComponents_[id].get();
    QObject::connect(component, SIGNAL(deleteComponent(QString)), linkObject, SLOT(deleteCardSpecifier(QString)));
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setCardSpecifier, this,
            [=](const asn::CardSpecifier& cardSpecifier, QString idParam) {
                auto *component = dynamic_cast<CardSpecifierComponent*>(cppComponents_[idParam].get());
                component->setCardSpecifier(cardSpecifier);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(component, SIGNAL(componentReady(asn::CardSpecifier,QString)), linkObject, SLOT(cardSpecifierChanged(asn::CardSpecifier,QString)));
    return component->getQmlObject();
}

QQuickItem* ComponentManager::createCard(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    cppComponents_[id] = std::make_unique<CardComponent>(langComponent.type, parent, id, langComponent.name);
    auto *component = cppComponents_[id].get();
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setCard, this,
            [=](const asn::Card& card, QString idParam) {
                auto *component = dynamic_cast<CardComponent*>(cppComponents_[idParam].get());
                component->setCard(card);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(component, SIGNAL(cardReady(asn::Card,QString)), mediator, SLOT(cardChanged(asn::Card,QString)));
    return component->getQmlObject();
}

QQuickItem* ComponentManager::createTargetAndPlace(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    cppComponents_[id] = std::make_unique<TargetAndPlaceComponent>(parent, id);
    auto *component = cppComponents_[id].get();
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setTargetAndPlace, this,
            [=](const asn::TargetAndPlace& target, QString idParam) {
                auto *component = dynamic_cast<TargetAndPlaceComponent*>(cppComponents_[idParam].get());
                component->setTargetAndPlace(target);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(component, SIGNAL(targetAndPlaceReady(asn::TargetAndPlace,QString)), mediator, SLOT(targetAndPlaceChanged(asn::TargetAndPlace,QString)));
    return component->getQmlObject();
}

QQuickItem* ComponentManager::createPlace(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    auto unique_component = std::make_unique<PlaceComponent>(parent, id, langComponent.name);
    auto *component = unique_component.get();
    cppComponents_[id] = std::move(unique_component);
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setPlace, this,
            [=](const asn::Place& place, QString idParam) {
                qWarning() << "comps:";
                for (auto &cppComponent: cppComponents_) {
                    qWarning() << cppComponent.first << " " << cppComponent.second->getComponentId();

                }
                auto *component = dynamic_cast<PlaceComponent*>(cppComponents_[idParam].get());
                component->setPlace(place);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(linkObject, &BaseComponent::placeTypeChanged, component, &PlaceComponent::onPlaceTypeChanged);
    QObject::connect(component, SIGNAL(placeReady(asn::Place,QString)), mediator, SLOT(placeChanged(asn::Place,QString)));
    return component->getQmlObject();
}

QQuickItem* ComponentManager::createNumber(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    cppComponents_[id] = std::make_unique<NumberComponent>(parent, id, langComponent.name);
    auto *component = cppComponents_[id].get();
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setNumber, this,
            [&, this](const asn::Number& number, QString idParam) {
                for (auto& comp: cppComponents_) {
                    qInfo() << comp.first;
                }
                auto *component = dynamic_cast<NumberComponent*>(cppComponents_[idParam].get());
                component->setNumber(number);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(component, SIGNAL(numberReady(asn::Number,QString)), mediator, SLOT(numberChanged(asn::Number,QString)));
    return component->getQmlObject();
}

QQuickItem* ComponentManager::createMultiplier(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    auto unique_component = std::make_unique<MultiplierComponent>(parent, id, langComponent.name);
    auto *component = unique_component.get();
    cppComponents_[id] = std::move(unique_component);
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setMultiplier, this,
            [=](const asn::Multiplier& multiplier, QString idParam) {
                auto *component = dynamic_cast<MultiplierComponent*>(cppComponents_[idParam].get());
                component->setMultiplier(multiplier);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(linkObject, &BaseComponent::valueTypeChanged, component, &MultiplierComponent::onValueTypeChanged);
    QObject::connect(component, SIGNAL(multiplierReady(asn::Multiplier,QString)), mediator, SLOT(multiplierChanged(asn::Multiplier,QString)));
    return component->getQmlObject();
}

QQuickItem* ComponentManager::createCondition(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator) {
    cppComponents_[id] = std::make_unique<ConditionComponent>(id, parent);
    auto *component = cppComponents_[id].get();
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setCondition, this,
            [=](const asn::Condition& condition, QString idParam) {
                auto *component = dynamic_cast<ConditionComponent*>(cppComponents_[idParam].get());
                component->setCondition(condition);
            });
        connections_.insert(connectionId, connection);
    }
    QObject::connect(component, SIGNAL(conditionReady(asn::Condition,QString)), mediator, SLOT(conditionChanged(asn::Condition,QString)));
    return component->getQmlObject();
}

QQuickItem *ComponentManager::createAbility(QString id, const LangComponent &langComponent, QQuickItem *parent, BaseComponent *linkObject, QObject *mediator) {
    auto abilityComponentCreator = [langComponent, id, parent, linkObject, mediator]() -> QQuickItem* {
        QQuickItem *object = createQmlObject(getBasicComponentQmlPath(langComponent.type), id, langComponent.name, parent->parentItem(), linkObject);
        QObject::connect(object, SIGNAL(componentChanged(asn::Ability,QString)), mediator, SLOT(abilityChanged(asn::Ability,QString)));
        return object;
    };
    cppComponents_[id] = std::make_unique<ComponentOpener>(parent, parent->parentItem(), id, abilityComponentCreator);
    auto *component = cppComponents_[id].get();
    QString connectionId = linkObject->getComponentId() + '/' + langComponent.type;
    if (!connections_.contains(connectionId)) {
        QMetaObject::Connection connection = QObject::connect(linkObject, &BaseComponent::setAbility, this,
            [=](const asn::Ability& ability, QString idParam) {
                auto *component = dynamic_cast<ComponentOpener*>(cppComponents_[idParam].get());
                component->initAbility(ability, idParam);
            });
    }
    return component->getQmlObject();
}

std::vector<QQuickItem*> ComponentManager::createCppComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator, int typeIndex, size_t arraySize) {
    std::vector<QQuickItem*> result;
    if (!creatorsMap_.contains(langComponent.type))
        throw "unhandled type";
    for (int i = 0; i < arraySize; ++i) {
        QString id = buildComponentId(langComponent.type, typeIndex, i);
        result.push_back((this->*creatorsMap_[langComponent.type])(id, langComponent, parent, linkObject, mediator));
    }
    if (langComponent.isArray) {
        QString arrayId = langComponent.type + QString::number(typeIndex);
        arrayComponents_[arrayId] = std::make_unique<ArrayComponent>(langComponent, arrayId, typeIndex, parent, linkObject, this, mediator, arraySize);
        result.push_back(arrayComponents_[arrayId].get()->getQmlObject());
    }
    return result;
}

QQuickItem* ComponentManager::createQmlComponentInternal(QString id, const LangComponent& langComponent, QQuickItem *parent, BaseComponent *linkObject, QObject *mediator) {
    QQuickItem *object = createQmlObject(getBasicComponentQmlPath(langComponent.type), id, langComponent.name, parent, linkObject);
    components_[id] = object;
    if (langComponent.type == "Zone") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(zoneChanged(QString,QString)));
    } else if (langComponent.type == "Order") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(orderChanged(QString,QString)));
    } else if (langComponent.type == "Phase") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(phaseChanged(QString,QString)));
    } else if (langComponent.type == "PhaseState") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(phaseStateChanged(QString,QString)));
    } else if (langComponent.type == "Player") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(playerChanged(QString,QString)));
    } else if (langComponent.type == "PlaceType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(placeTypeChanged(QString,QString)));
        if (mediator != linkObject) {
            QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SLOT(onPlaceTypeChanged(QString,QString)));
        }
    } else if (langComponent.type == "Position") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(positionChanged(QString,QString)));
    } else if (langComponent.type == "State") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(stateChanged(QString,QString)));
    } else if (langComponent.type == "TiggerIcon") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(tiggerIconChanged(QString,QString)));
    } else if (langComponent.type == "AttackType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(attackTypeChanged(QString,QString)));
    } else if (langComponent.type == "AbilityType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(abilityTypeChanged(QString,QString)));
    } else if (langComponent.type == "AttributeType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(attributeTypeChanged(QString,QString)));
    } else if (langComponent.type == "ValueType") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(valueTypeChanged(QString,QString)));
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), linkObject, SIGNAL(valueTypeChanged(QString,QString)));
    } else if (langComponent.type == "Bool") {
        QObject::connect(object, SIGNAL(valueChanged(bool,QString)), mediator, SLOT(boolChanged(bool,QString)));
    } else if (langComponent.type == "String") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(stringChanged(QString,QString)));
    } else if (langComponent.type == "Int32") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(int32Changed(QString,QString)));
    } else if (langComponent.type == "Int8") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(int8Changed(QString,QString)));
    } else if (langComponent.type == "UInt8") {
        QObject::connect(object, SIGNAL(valueChanged(QString,QString)), mediator, SLOT(uInt8Changed(QString,QString)));
    } else if (langComponent.type == "Duration") {
        QObject::connect(object, SIGNAL(valueChanged(int,QString)), mediator, SLOT(durationChanged(int,QString)));
    } else {
        qWarning() << "did not find component to create";
    }
    return object;
}

std::vector<QQuickItem*> ComponentManager::createQmlComponent(const LangComponent& langComponent, QQuickItem *parent, BaseComponent *linkObject, QObject *mediator, int typeIndex, size_t arraySize) {
    std::vector<QQuickItem*> result;
    for (int i = 0; i < arraySize; ++i) {
        QString id = buildComponentId(langComponent.type, typeIndex, i);
        auto object = createQmlComponentInternal(id, langComponent, parent, linkObject, mediator);
        result.push_back(object);
    }
    return result;
}

ComponentManager::ComponentManager() {
    creatorsMap_ = std::unordered_map<QString, MemberFunctionPtr>{
        {"Target", &ComponentManager::createTarget},
        {"TargetAndPlace", &ComponentManager::createTargetAndPlace},
        {"Number", &ComponentManager::createNumber},
        {"Place", &ComponentManager::createPlace},
        {"Condition", &ComponentManager::createCondition},
        {"Multiplier", &ComponentManager::createMultiplier},
        {"Card", &ComponentManager::createCard},
        {"CardSpecifier", &ComponentManager::createCardSpecifier},
        {"Ability", &ComponentManager::createAbility},
    };
}

ComponentManager::~ComponentManager() {
    clear();
}

void ComponentManager::clear() {
    for (auto &component: qAsConst(components_)) {
        component->deleteLater();
    }
    for (auto &connection: qAsConst(connections_)) {
        QObject::disconnect(connection);
    }
    arrayComponents_.clear();
    components_.clear();
    cppComponents_.clear();
    connections_.clear();
}

QString ComponentManager::buildComponentId(QString type, int typeIndex, int arrayPosition) {
    return type + '/' + QString::number(typeIndex) + '/' + QString::number(arrayPosition);
}

void ComponentManager::deleteComponent(QString id) {
    if (cppComponents_.contains(id)) {
        cppComponents_.erase(id);
    } else if (components_.contains(id)) {
        components_[id]->deleteLater();
        components_.remove(id);
    }
}
