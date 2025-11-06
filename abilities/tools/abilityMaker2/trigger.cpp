#include "trigger.h"

#include <set>

#include <QQmlContext>

#include "componentHelper.h"
#include "languageSpecification.h"
#include "language_parser.h"
#include "triggerInit.h"


TriggerComponent::TriggerComponent(QQuickItem *parent, QQuickItem *abilityMaker)
    : BaseComponent("Trigger", parent) {
    init(parent, abilityMaker);
}

TriggerComponent::TriggerComponent(QQuickItem *parent, QQuickItem *abilityMaker, const asn::Trigger& trigger)
    : BaseComponent("Trigger", parent) {
    init(parent, abilityMaker);
    type_ = trigger.type;
    trigger_ = trigger.trigger;
    QMetaObject::invokeMethod(qmlObject, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createTrigger();
}

void TriggerComponent::init(QQuickItem *parent, QQuickItem *abilityMaker) {
    abilityMaker_ = abilityMaker;
    qvariant_cast<QObject*>(qmlObject->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    connect(qmlObject, SIGNAL(triggerTypeChanged(QString)), this, SLOT(onTriggerTypeChanged(QString)));
}

TriggerComponent::~TriggerComponent() {
    for (auto *component: components_) {
        component->deleteLater();
    }
}

void TriggerComponent::fitComponent(QQuickItem* object) {
    if (components_.empty()) {
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", qmlObject->property("left"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(50));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", qmlObject->property("top"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("topMargin", QVariant(130));
    } else {
        auto *lastObject = components_.back();
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", lastObject->property("right"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(10));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", lastObject->property("top"));
    }
    components_.push_back(object);
}

void TriggerComponent::onTriggerTypeChanged(QString type) {
    qDebug() << "changing trigger type";
    type_ = parse(type.toStdString(), formats::To<asn::TriggerType>{});
    trigger_ = getDefaultTrigger(type_);
    createTrigger();
}


void TriggerComponent::createTrigger() {
    componentManager_.clear();
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponents(QString::fromStdString(toString(type_)));
    std::set<std::string> types;
    for (const auto& comp: components) {
        QString component_id = comp.type + (types.contains(comp.type.toStdString()) ? QString("2") : QString(""));
        types.insert(comp.type.toStdString());
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject, this);
        fitComponent(object);
    }

    setTriggerInQml();
}

void TriggerComponent::zoneChanged(QString value, QString componentId) {
    if (componentId == "Zone2") {
        switch (type_) {
        case asn::TriggerType::OnZoneChange: {
            auto &trig = std::get<asn::ZoneChangeTrigger>(trigger_);
            trig.to = parse(value.toStdString(), formats::To<asn::Zone>{});
            break;
        }
        default:
            throw std::logic_error("zone2Changed");
        }
        return;
    }
    switch (type_) {
    case asn::TriggerType::OnZoneChange: {
        auto &trig = std::get<asn::ZoneChangeTrigger>(trigger_);
        trig.from = parse(value.toStdString(), formats::To<asn::Zone>{});
        break;
    }
    default:
        throw std::logic_error("zoneChanged");
    }
}

void TriggerComponent::targetChanged(const asn::Target& target) {
    switch(type_) {
    case asn::TriggerType::OnZoneChange:{
        auto &trig = std::get<asn::ZoneChangeTrigger>(trigger_);
        trig.target.clear();
        trig.target.push_back(target);
        break;
    }
    case asn::TriggerType::OnPlay:{
        auto &trig = std::get<asn::OnPlayTrigger>(trigger_);
        trig.target = target;
        break;
    }
    case asn::TriggerType::OnAttack:{
        auto &trig = std::get<asn::OnAttackTrigger>(trigger_);
        trig.target = target;
        break;
    }
    case asn::TriggerType::OnStateChange:{
        auto &trig = std::get<asn::StateChangeTrigger>(trigger_);
        trig.target = target;
        break;
    }
    case asn::TriggerType::OnBeingAttacked:{
        auto &trig = std::get<asn::OnBeingAttackedTrigger>(trigger_);
        trig.target = target;
        break;
    }
    case asn::TriggerType::OnDamageCancel:{
        auto &trig = std::get<asn::OnDamageCancelTrigger>(trigger_);
        trig.damageDealer = target;
        break;
    }
    case asn::TriggerType::OnPayingCost:{
        auto &trig = std::get<asn::OnPayingCostTrigger>(trigger_);
        trig.target = target;
        break;
    }
    default: throw std::logic_error("unhandled target in trigger");
    }
}

void TriggerComponent::phaseChanged(QString value, QString componentId) {
    switch(type_) {
    case asn::TriggerType::OnPhaseEvent: {
        auto &trig = std::get<asn::PhaseTrigger>(trigger_);
        trig.phase = parse(value.toStdString(), formats::To<asn::Phase>{});
        break;
    }
    default:
        throw std::logic_error("unhandled Phase");
    }
}

void TriggerComponent::phaseStateChanged(QString value, QString componentId) {
    switch(type_) {
    case asn::TriggerType::OnPhaseEvent: {
        auto &trig = std::get<asn::PhaseTrigger>(trigger_);
        trig.state = parse(value.toStdString(), formats::To<asn::PhaseState>{});
        break;
    }
    default:
        throw std::logic_error("unhandled PhaseState");
    }
}

void TriggerComponent::playerChanged(QString value, QString componentId) {
    switch(type_) {
    case asn::TriggerType::OnPhaseEvent: {
        auto &trig = std::get<asn::PhaseTrigger>(trigger_);
        trig.player = parse(value.toStdString(), formats::To<asn::Player>{});
        break;
    }
    case asn::TriggerType::OnActAbillity: {
        auto &trig = std::get<asn::OnActAbillityTrigger>(trigger_);
        trig.player = parse(value.toStdString(), formats::To<asn::Player>{});
        break;
    }
    default:
        throw std::logic_error("unhandled Player");
    }
}

void TriggerComponent::stateChanged(QString value, QString componentId) {
    switch(type_) {
    case asn::TriggerType::OnStateChange: {
        auto &trig = std::get<asn::StateChangeTrigger>(trigger_);
        trig.state = parse(value.toStdString(), formats::To<asn::State>{});
        break;
    }
    default:
        throw std::logic_error("unhandled State");
    }
}

void TriggerComponent::attackTypeChanged(QString value, QString componentId) {
    switch(type_) {
    case asn::TriggerType::OnBeingAttacked: {
        auto &trig = std::get<asn::OnBeingAttackedTrigger>(trigger_);
        trig.attackType = parse(value.toStdString(), formats::To<asn::AttackType>{});
        break;
    }
    default:
        throw std::logic_error("unhandled AttackType");
    }
}

void TriggerComponent::boolChanged(bool value, QString componentId) {
    switch(type_) {
    case asn::TriggerType::OnDamageCancel: {
        auto &trig = std::get<asn::OnDamageCancelTrigger>(trigger_);
        trig.cancelled = value;
        break;
    }
    case asn::TriggerType::OnDamageTakenCancel: {
        auto &trig = std::get<asn::OnDamageTakenCancelTrigger>(trigger_);
        trig.cancelled = value;
        break;
    }
    default:
        throw std::logic_error("unhandled bool");
    }
}

void TriggerComponent::abilityTypeChanged(QString value, QString componentId) {
    switch(type_) {
    case asn::TriggerType::OnPayingCost: {
        auto &trig = std::get<asn::OnPayingCostTrigger>(trigger_);
        trig.abilityType = parse(value.toStdString(), formats::To<asn::AbilityType>{});
        break;
    }
    default:
        throw std::logic_error("unhandled AbilityType");
    }
}
