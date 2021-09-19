#include "targetComponent.h"

#include <QQmlContext>

TargetComponent::TargetComponent(QQuickItem *parent)
    : BaseComponent("Target", parent) {
    init();
}

TargetComponent::TargetComponent(const asn::Target &t, QQuickItem *parent)
    : BaseComponent("Target", parent) {
    init();
    initTarget(t);
}

void TargetComponent::init() {
    connect(qmlObject, SIGNAL(presetChanged(int)), this, SLOT(updateTarget(int)));

    connect(qmlObject, SIGNAL(typeChanged(int)), this, SLOT(typeChanged(int)));
    connect(qmlObject, SIGNAL(targetModeChanged(int)), this, SLOT(targetModeChanged(int)));
    connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(numModifierChanged(int)));
    connect(qmlObject, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged(QString)));
    connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
    connect(qmlObject, SIGNAL(clearCard()), this, SLOT(clearCard()));

    connect(this, SIGNAL(initTargetType(int)), qmlObject, SIGNAL(incomingTargetType(int)));
    connect(this, SIGNAL(initTargetMode(int)), qmlObject, SIGNAL(incomingTargetMode(int)));
    connect(this, SIGNAL(initNumModifier(int)), qmlObject, SIGNAL(incomingNumModifier(int)));
    connect(this, SIGNAL(initNumValue(QString)), qmlObject, SIGNAL(incomingNumValue(QString)));

    // set y after setting the parent
    QMetaObject::invokeMethod(qmlObject, "setActualY");

    number.mod = asn::NumModifier::ExactMatch;
}

void TargetComponent::initTarget(const asn::Target &t) {
    emit initTargetType(static_cast<int>(t.type));
    if (t.type == asn::TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        emit initTargetMode(static_cast<int>(spec.mode));
        emit initNumModifier(static_cast<int>(spec.number.mod));
        emit initNumValue(QString::number(spec.number.value));
        card = spec.cards;
        cardSet = true;
    }
}

void TargetComponent::componentReady() {
    emit componentChanged(constructTarget());
    emit close();
}

void TargetComponent::updateTarget(int index) {

}

void TargetComponent::typeChanged(int index) {
    type = static_cast<asn::TargetType>(index);
}

void TargetComponent::targetModeChanged(int index) {
    mode = static_cast<asn::TargetMode>(index);
}

void TargetComponent::numModifierChanged(int index) {
    number.mod = static_cast<asn::NumModifier>(index);
}

void TargetComponent::valueChanged(QString value) {
    number.value = value.toInt();
}

void TargetComponent::editCard() {
    if (cardSet)
        qmlCard = std::make_unique<CardComponent>(card, qmlObject);
    else
        qmlCard = std::make_unique<CardComponent>(qmlObject);

    connect(qmlCard.get(), &CardComponent::componentChanged, this, &TargetComponent::cardReady);
    connect(qmlCard.get(), &CardComponent::close, this, &TargetComponent::destroyCard);
}

void TargetComponent::clearCard() {
    card = asn::Card();
}

void TargetComponent::cardReady(const asn::Card &card_) {
    card = card_;
    cardSet = true;
    emit componentChanged(constructTarget());
}

void TargetComponent::destroyCard() {
    qmlCard.reset();
}

asn::Target TargetComponent::constructTarget() {
    asn::Target t;
    t.type = type;
    if (type == asn::TargetType::SpecificCards) {
        t.targetSpecification = asn::TargetSpecificCards();
        t.targetSpecification->mode = mode;
        t.targetSpecification->number = number;
        t.targetSpecification->cards = card;
    }
    return t;
}
