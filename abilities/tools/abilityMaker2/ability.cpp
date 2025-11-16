#include "ability.h"

#include "trigger.h"
#include "triggerInit.h"
#include "language_parser.h"

void AbilityComponent::componentComplete() {
    QQuickItem::componentComplete();
}

void AbilityComponent::createTrigger(QString triggerId, QQuickItem *parent) {
    auto trigger = getTriggerFromPreset(triggerId);
    if (trigger.has_value()) {
        QMetaObject::invokeMethod(this, "setTriggerText", Q_ARG(QString, QString::fromStdString(toString(trigger.value().type))));
        triggers_.clear();
        triggers_.push_back(trigger.value());
        openTrigger(parent);
    } else {
        currentComponent = std::make_shared<TriggerComponent>(parent, this);
    }
}

void AbilityComponent::openTrigger(QQuickItem *parent) {
    auto triggerComponent = std::make_shared<TriggerComponent>(parent, this, triggers_);
    connect(&*triggerComponent, &TriggerComponent::componentChanged, this, &AbilityComponent::triggersChanged);
    currentComponent = triggerComponent;
}

void AbilityComponent::triggersChanged(const std::vector<asn::Trigger>& triggers) {
    auto triggerText = QString::fromStdString(toString(triggers.at(0).type));
    if (triggers.size() > 1) triggerText =+ "...";
    QMetaObject::invokeMethod(this, "setTriggerText", Q_ARG(QString, triggerText));
    triggers_ = triggers;

    emit componentChanged(constructAbility());
}

asn::Ability AbilityComponent::constructAbility() {
    asn::Ability ability;
    ability.type = type_;
    switch (type_) {
    case asn::AbilityType::Auto:
        ability.ability = constructSpecificAbility<asn::AutoAbility>();
        break;
    case asn::AbilityType::Cont:
        ability.ability = constructSpecificAbility<asn::ContAbility>();
        break;
    case asn::AbilityType::Act:
        ability.ability = constructSpecificAbility<asn::ActAbility>();
        break;
    case asn::AbilityType::Event:
        ability.ability = constructSpecificAbility<asn::EventAbility>();
        break;
    }
    return ability;
}
