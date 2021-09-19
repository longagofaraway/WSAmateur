#include "arrayOfEffectsComponent.h"

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

ArrayOfEffectsComponent::ArrayOfEffectsComponent(QQuickItem *parent)
    : BaseComponent("ArrayOfEffects", parent) {
    init();
}

ArrayOfEffectsComponent::ArrayOfEffectsComponent(const std::vector<asn::Effect> &ef, QQuickItem *parent)
    : BaseComponent("ArrayOfEffects", parent) {
    init();

    effects = ef;
    for (size_t i = 0; i < ef.size(); ++i) {
        createEffect();
        effectSet.push_back(true);
    }
}

void ArrayOfEffectsComponent::init() {
    qmlObject->setProperty("kOffset", kElemOffset);
    connect(qmlObject, SIGNAL(addEffect()), this, SLOT(addEffect()));

    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void ArrayOfEffectsComponent::addEffect() {
    createEffect();
    effects.push_back(asn::Effect());
    effectSet.push_back(false);
    conditionSet.push_back(false);
}

void ArrayOfEffectsComponent::editEffect(int pos) {
    if (effectSet[pos])
        qmlEffect = std::make_unique<EffectComponent>(effects[pos], qmlObject);
    else
        qmlEffect = std::make_unique<EffectComponent>(qmlObject);

    currentPos = pos;
    connect(qmlEffect.get(), &EffectComponent::componentChanged, this, &ArrayOfEffectsComponent::effectReady);
    connect(qmlEffect.get(), &EffectComponent::close, this, &ArrayOfEffectsComponent::destroyEffect);
}

void ArrayOfEffectsComponent::editCondition(int pos) {
    if (conditionSet[pos])
        qmlCondition = std::make_unique<ConditionComponent>(effects[pos].cond, qmlObject);
    else
        qmlCondition = std::make_unique<ConditionComponent>(qmlObject);

    currentPos = pos;
    connect(qmlCondition.get(), &ConditionComponent::componentChanged, this, &ArrayOfEffectsComponent::conditionReady);
    connect(qmlCondition.get(), &ConditionComponent::close, this, &ArrayOfEffectsComponent::destroyCondition);
}

void ArrayOfEffectsComponent::destroyEffect() {
    qmlEffect.reset();
}

void ArrayOfEffectsComponent::effectReady(const asn::Effect &effect) {
    effects[currentPos] = effect;
    effectSet[currentPos] = true;
    emit componentChanged(effects);
}

void ArrayOfEffectsComponent::destroyCondition() {
    qmlCondition.reset();
}

void ArrayOfEffectsComponent::conditionReady(const asn::Condition &condition) {
    effects[currentPos].cond = condition;
    conditionSet[currentPos] = true;
    emit componentChanged(effects);
}

void ArrayOfEffectsComponent::componentReady() {
    emit componentChanged(effects);
    emit close();
}

void ArrayOfEffectsComponent::createEffect() {
    auto obj = createQmlObject("effects/EffectButton", qmlObject);
    obj->setProperty("position", QVariant((int)qmlEffects.size()));
    obj->setProperty("x", QVariant((int)(kElemOffset * qmlEffects.size() + 10)));
    obj->setProperty("y", 100);
    qmlEffects.push_back(obj);
    connect(obj, SIGNAL(editEffect(int)), this, SLOT(editEffect(int)));
    connect(obj, SIGNAL(editCondition(int)), this, SLOT(editCondition(int)));
    qmlObject->setProperty("specCount", qmlObject->property("specCount").toInt() + 1);
}
