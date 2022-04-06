#include "arrayOfAbilitiesComponent.h"

#include <QQmlContext>

#include "abilityComponent.h"

namespace {
int kElemOffset = 140;
QQuickItem* createQmlObject(const QString &name, QQuickItem *parent) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    return qmlObject;
}
}

ArrayOfAbilitiesComponent::ArrayOfAbilitiesComponent(QQuickItem *parent)
    : BaseComponent("ArrayOfAbilities", parent, "abilities") {
    init();
}

ArrayOfAbilitiesComponent::ArrayOfAbilitiesComponent(const std::vector<asn::Ability> &ab, QQuickItem *parent)
    : BaseComponent("ArrayOfAbilities", parent, "abilities") {
    init();

    abilities = ab;
    for (size_t i = 0; i < ab.size(); ++i) {
        createAbility();
        abilitiesSet.push_back(true);
    }
}

void ArrayOfAbilitiesComponent::init() {
    qmlObject->setProperty("kOffset", kElemOffset);
    connect(qmlObject, SIGNAL(addAbility()), this, SLOT(addAbility()));

    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void ArrayOfAbilitiesComponent::addAbility() {
    createAbility();
    abilities.push_back(asn::Ability());
    abilitiesSet.push_back(false);
}

void ArrayOfAbilitiesComponent::editAbility(int pos) {
    if (abilitiesSet[pos])
        qmlAbility = std::make_shared<AbilityComponent>(abilities[pos], qmlObject, pos);
    else
        qmlAbility = std::make_shared<AbilityComponent>(qmlObject, pos);
    if (onlyEventAbilities)
        qmlAbility->fixEventAbility();

    currentPos = pos;
    connect(qmlAbility.get(), &AbilityComponent::componentChanged, this, &ArrayOfAbilitiesComponent::abilityReady);
    connect(qmlAbility.get(), &AbilityComponent::close, this, &ArrayOfAbilitiesComponent::destroyAbility);
}

void ArrayOfAbilitiesComponent::destroyAbility() {
    qmlAbility.reset();
}

void ArrayOfAbilitiesComponent::abilityReady(const asn::Ability &ability) {
    abilities[currentPos] = ability;
    abilitiesSet[currentPos] = true;
    emit componentChanged(abilities);
}

void ArrayOfAbilitiesComponent::componentReady() {
    emit componentChanged(abilities);
    emit close();
}

void ArrayOfAbilitiesComponent::createAbility() {
    auto obj = createQmlObject("effects/AbilityButton", qmlObject);
    obj->setProperty("position", QVariant((int)qmlAbilities.size()));
    obj->setProperty("x", QVariant((int)(kElemOffset * qmlAbilities.size() + 10)));
    obj->setProperty("y", 100);
    qmlAbilities.push_back(obj);
    connect(obj, SIGNAL(editAbility(int)), this, SLOT(editAbility(int)));
    qmlObject->setProperty("specCount", qmlObject->property("specCount").toInt() + 1);
}
