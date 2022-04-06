#include "arrayOfConditionsComponent.h"

#include <QQmlContext>

#include "conditionComponent.h"

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

ArrayOfConditionsComponent::ArrayOfConditionsComponent(QQuickItem *parent)
    : BaseComponent("ArrayOfConditions", parent, "conditions") {
    init();
}

ArrayOfConditionsComponent::ArrayOfConditionsComponent(const std::vector<asn::Condition> &ab, QQuickItem *parent)
    : BaseComponent("ArrayOfConditions", parent, "conditions") {
    init();

    conditions = ab;
    for (size_t i = 0; i < ab.size(); ++i) {
        createCondition();
        conditionsSet.push_back(true);
    }
}

void ArrayOfConditionsComponent::init() {
    qmlObject->setProperty("kOffset", kElemOffset);
    connect(qmlObject, SIGNAL(addCondition()), this, SLOT(addCondition()));

    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void ArrayOfConditionsComponent::addCondition() {
    createCondition();
    conditions.push_back(asn::Condition());
    conditionsSet.push_back(false);
}

void ArrayOfConditionsComponent::editCondition(int pos) {
    if (conditionsSet[pos])
        qmlCondition = std::make_shared<ConditionComponent>(conditions[pos], qmlObject, pos);
    else
        qmlCondition = std::make_shared<ConditionComponent>(qmlObject, pos);

    currentPos = pos;
    connect(qmlCondition.get(), &ConditionComponent::componentChanged, this, &ArrayOfConditionsComponent::conditionReady);
    connect(qmlCondition.get(), &ConditionComponent::close, this, &ArrayOfConditionsComponent::destroyCondition);
}

void ArrayOfConditionsComponent::destroyCondition() {
    qmlCondition.reset();
}

void ArrayOfConditionsComponent::conditionReady(const asn::Condition &condition) {
    conditions[currentPos] = condition;
    conditionsSet[currentPos] = true;
    emit componentChanged(conditions);
}

void ArrayOfConditionsComponent::componentReady() {
    emit componentChanged(conditions);
    emit close();
}

void ArrayOfConditionsComponent::createCondition() {
    auto obj = createQmlObject("conditions/ConditionButton", qmlObject);
    obj->setProperty("position", QVariant((int)qmlConditions.size()));
    obj->setProperty("x", QVariant((int)(kElemOffset * qmlConditions.size() + 10)));
    obj->setProperty("y", 100);
    qmlConditions.push_back(obj);
    connect(obj, SIGNAL(editCondition(int)), this, SLOT(editCondition(int)));
    qmlObject->setProperty("specCount", qmlObject->property("specCount").toInt() + 1);
}
