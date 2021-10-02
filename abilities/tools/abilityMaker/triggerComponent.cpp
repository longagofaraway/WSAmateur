#include "triggerComponent.h"

#include <QQmlContext>

TriggerComponent::TriggerComponent(QQuickItem *parent)
    : BaseComponent("Trigger", parent)  {
    init(parent);
}

TriggerComponent::TriggerComponent(const asn::Trigger &trigger, QQuickItem *parent)
    : BaseComponent("Trigger", parent)  {
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

void TriggerComponent::fromHandToStagePreset() {
    auto tr = asn::ZoneChangeTrigger();
    tr.from = asn::Zone::Hand;
    tr.to = asn::Zone::Stage;
    auto targ = asn::Target();
    targ.type = asn::TargetType::ThisCard;
    tr.target.push_back(targ);

    type = asn::TriggerType::OnZoneChange;
    trigger = tr;
    initializing = true;
    emit passTriggerType((int)type);
}

void TriggerComponent::thisCardAttacks() {
    auto tr = asn::OnAttackTrigger();
    auto targ = asn::Target();
    targ.type = asn::TargetType::ThisCard;
    tr.target = targ;

    type = asn::TriggerType::OnAttack;
    trigger = tr;
    initializing = true;
    emit passTriggerType((int)type);
}

void TriggerComponent::thisCardBecomesReversed() {
    auto tr = asn::StateChangeTrigger();
    auto targ = asn::Target();
    targ.type = asn::TargetType::ThisCard;
    tr.target = targ;
    tr.state = asn::State::Reversed;

    type = asn::TriggerType::OnStateChange;
    trigger = tr;
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
    case asn::TriggerType::OnEndOfThisTurn:
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

void TriggerComponent::updateTrigger(int index) {
    switch (index) {
    case 0:
        fromHandToStagePreset();
        break;
    case 1:
        thisCardAttacks();
        break;
    case 2:
        thisCardBecomesReversed();
        break;
    }
}

void TriggerComponent::onTriggerChanged(const VarTrigger &t) {
    trigger = t;
    emit componentChanged(constructTrigger());
}
