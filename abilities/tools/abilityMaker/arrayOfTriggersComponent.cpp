#include "arrayOfTriggersComponent.h"

#include "statusLine.h"

#include <QQmlContext>

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

ArrayOfTriggersComponent::ArrayOfTriggersComponent(QQuickItem *parent)
    : BaseComponent("ArrayOfTriggers", parent, "triggers") {
    init();
}

ArrayOfTriggersComponent::ArrayOfTriggersComponent(const std::vector<asn::Trigger> &tr, QQuickItem *parent)
    : BaseComponent("ArrayOfTriggers", parent, "triggers") {
    init();

    triggers = tr;
    for (size_t i = 0; i < tr.size(); ++i) {
        createTrigger();
    }
}

void ArrayOfTriggersComponent::init() {
    defaultTrigger.type = asn::TriggerType::NotSpecified;

    qmlObject->setProperty("kOffset", kElemOffset);
    connect(qmlObject, SIGNAL(addTrigger()), this, SLOT(addTrigger()));

    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void ArrayOfTriggersComponent::addTrigger() {
    createTrigger();
    triggers.push_back(defaultTrigger);
}

void ArrayOfTriggersComponent::editTrigger(int pos) {
    if (triggers[pos].type != asn::TriggerType::NotSpecified)
        qmlTrigger = std::make_unique<TriggerComponent>(triggers[pos], qmlObject, pos);
    else
        qmlTrigger = std::make_unique<TriggerComponent>(qmlObject, pos);

    currentPos = pos;
    connect(qmlTrigger.get(), &TriggerComponent::componentChanged, this, &ArrayOfTriggersComponent::triggerReady);
    connect(qmlTrigger.get(), &TriggerComponent::close, this, &ArrayOfTriggersComponent::destroyTrigger);
}

void ArrayOfTriggersComponent::destroyTrigger() {
    qmlTrigger.reset();
}

void ArrayOfTriggersComponent::triggerReady(const asn::Trigger &trigger) {
    triggers[currentPos].type = trigger.type;
    triggers[currentPos].trigger = trigger.trigger;
    emit componentChanged(triggers);
}

void ArrayOfTriggersComponent::componentReady() {
    if (std::any_of(triggers.begin(), triggers.end(), [](const auto &t){
        return t.type == asn::TriggerType::NotSpecified;
    }))
        return;
    emit componentChanged(triggers);
    emit close();
}

void ArrayOfTriggersComponent::createTrigger() {
    auto obj = createQmlObject("triggers/TriggerButton", qmlObject);
    obj->setProperty("position", QVariant((int)qmlTriggers.size()));
    obj->setProperty("x", QVariant((int)(kElemOffset * qmlTriggers.size() + 10)));
    obj->setProperty("y", 100);
    qmlTriggers.push_back(obj);
    connect(obj, SIGNAL(editTrigger(int)), this, SLOT(editTrigger(int)));
    qmlObject->setProperty("specCount", qmlObject->property("specCount").toInt() + 1);
}
