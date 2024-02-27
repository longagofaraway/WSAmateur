#include "triggerComponent.h"

#include <QQmlContext>

#include "hardcodedAbilities.h"

TriggerComponent::TriggerComponent(QQuickItem *parent, int position)
    : BaseComponent("Trigger", parent, "trigger" + QString::number(position)) {
    init(parent);
}

TriggerComponent::TriggerComponent(const asn::Trigger &trigger, QQuickItem *parent, int position)
    : BaseComponent("Trigger", parent, "trigger" + QString::number(position)) {
    init(parent);
    initTrigger(trigger);
}

TriggerComponent::~TriggerComponent() {
    qmlObject->deleteLater();
}

void TriggerComponent::init(QQuickItem *parent){
    connect(qmlObject, SIGNAL(presetChanged(int)), this, SLOT(updateTrigger(int)));
    connect(qmlObject, SIGNAL(triggerTypeChanged(int)), this, SLOT(setTriggerType(int)));

    connect(this, SIGNAL(passTriggerType(int)), qmlObject, SIGNAL(incomingTriggerType(int)));
}

void TriggerComponent::initTrigger(const asn::Trigger &_trigger) {
    initializing = true;
    type = _trigger.type;
    trigger = _trigger.trigger;
    emit passTriggerType((int)type);
}

asn::Trigger TriggerComponent::constructTrigger() {
    asn::Trigger t;
    t.type = type;
    t.trigger = trigger;
    return t;
}

void TriggerComponent::updateTrigger(int index) {
    asn::Trigger builtTrigger;
    switch (index) {
    case 0:
        builtTrigger = fromHandToStagePreset();
        break;
    case 1:
        builtTrigger = thisCardAttacks();
        break;
    case 2:
        builtTrigger = thisCardBecomesReversed();
        break;
    case 3:
        builtTrigger = climaxIsPlaced();
        break;
    }
    type = builtTrigger.type;
    trigger = builtTrigger.trigger;
    initializing = true;
    emit passTriggerType((int)type);
}

void TriggerComponent::componentReady() {
    emit componentChanged(constructTrigger());
    emit close();
}

void TriggerComponent::setTriggerType(int index) {
    bool needImpl = true;
    type = static_cast<asn::TriggerType>(index);
    switch (type) {
    case asn::TriggerType::OnBackupOfThis:
    case asn::TriggerType::OnEndOfThisCardsAttack:
    case asn::TriggerType::OnOppCharPlacedByStandbyTriggerReveal:
    case asn::TriggerType::OtherTrigger:
        needImpl = false;
    default:
        break;
    }

    if (initializing) {
        if (needImpl)
            qmlTriggerImpl = std::make_unique<TriggerImplComponent>(type, trigger, qmlObject);
        else
            qmlTriggerImpl.reset();
        initializing = false;
        emit componentChanged(constructTrigger());
    } else {
        type = static_cast<asn::TriggerType>(index);

        if (needImpl) {
            qmlTriggerImpl = std::make_unique<TriggerImplComponent>(type, qmlObject);
            initTriggerByType(trigger, type);
        } else {
            qmlTriggerImpl.reset();
        }
    }

    if (needImpl)
        connect(qmlTriggerImpl.get(), &TriggerImplComponent::componentChanged, this, &TriggerComponent::onTriggerChanged);
}

void TriggerComponent::onTriggerChanged(const VarTrigger &t) {
    trigger = t;
    emit componentChanged(constructTrigger());
}
