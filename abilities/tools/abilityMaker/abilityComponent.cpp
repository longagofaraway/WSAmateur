#include "abilityComponent.h"

#include <QQmlContext>

namespace {
asn::Keyword stringToKeyword(const QString &s) {
    std::unordered_map<std::string, asn::Keyword> map{{"Encore", asn::Keyword::Encore},
                                                  {"CxCombo", asn::Keyword::Cxcombo},
                                                  {"Brainstorm", asn::Keyword::Brainstorm},
                                                  {"Backup", asn::Keyword::Backup},
                                                  {"Experience", asn::Keyword::Experience},
                                                  {"Resonance", asn::Keyword::Resonance},
                                                  {"Bond", asn::Keyword::Bond},
                                                  {"Replay", asn::Keyword::Replay},
                                                  {"Alarm", asn::Keyword::Alarm},
                                                  {"Change", asn::Keyword::Change},
                                                  {"Assist", asn::Keyword::Assist}};
    return map[s.toStdString()];
}
}

AbilityComponent::AbilityComponent(QQuickItem *parent)
    : BaseComponent("Ability", parent) {
    init();
}

AbilityComponent::AbilityComponent(const asn::Ability &a, QQuickItem *parent)
    : BaseComponent("Ability", parent) {
    init();

    //QMetaObject::invokeMethod(qmlObject, "changeAbilityType", Q_ARG(QVariant, (int)a.type));
}

void AbilityComponent::init() {
    trigger.type = asn::TriggerType::NotSpecified;
    connect(qmlObject, SIGNAL(setAbilityType(int)), this, SLOT(setAbilityType(int)));
    connect(qmlObject, SIGNAL(setActivationTimes(int)), this, SLOT(setActivationTimes(int)));
    connect(qmlObject, SIGNAL(setKeywords(QVariant)), this, SLOT(setKeywords(QVariant)));
    connect(qmlObject, SIGNAL(editTrigger()), this, SLOT(editTrigger()));
    connect(qmlObject, SIGNAL(editEffects()), this, SLOT(editEffects()));
    connect(qmlObject, SIGNAL(editCost()), this, SLOT(editCost()));

    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void AbilityComponent::removeButtons() {
    qmlObject->setProperty("noButtons", true);
}

void AbilityComponent::fixEventAbility() {
    QMetaObject::invokeMethod(qmlObject, "fixAbilityType", Q_ARG(QVariant, (int)asn::AbilityType::Event));
}

void AbilityComponent::addDbControls(AbilityMaker *maker) {
    dbControls = std::make_unique<DbControls>(maker, qmlObject);
}

void AbilityComponent::setAbilityType(int type_) {
    type = static_cast<asn::AbilityType>(type_);
    emit componentChanged(constructAbility());
}

void AbilityComponent::setActivationTimes(int times) {
    activationTimes = times;
    emit componentChanged(constructAbility());
}

void AbilityComponent::setKeywords(QVariant keywordList) {
    keywords.clear();
    auto list = keywordList.toList();
    for (int i = 0; i < list.size(); ++i)
        keywords.push_back(stringToKeyword(list[i].toString()));
    emit componentChanged(constructAbility());
}

void AbilityComponent::editTrigger() {
    if (triggerSet)
        qmlTrigger = std::make_unique<TriggerComponent>(trigger, qmlObject);
    else
        qmlTrigger = std::make_unique<TriggerComponent>(qmlObject);

    connect(qmlTrigger.get(), &TriggerComponent::componentChanged, this, &AbilityComponent::triggerReady);
    connect(qmlTrigger.get(), &TriggerComponent::close, this, &AbilityComponent::destroyTrigger);
}

void AbilityComponent::triggerReady(const asn::Trigger &t) {
    triggerSet = true;
    trigger = t;
    emit componentChanged(constructAbility());
}

void AbilityComponent::destroyTrigger() {
    qmlTrigger.reset();
}

void AbilityComponent::destroyEffects() {
    qmlEffects.reset();
}

void AbilityComponent::destroyCost() {
    qmlCost.reset();
}

void AbilityComponent::editCost() {
    if (cost.has_value())
        qmlCost = std::make_unique<CostComponent>(*cost, qmlObject);
    else
        qmlCost = std::make_unique<CostComponent>(qmlObject);

    connect(qmlCost.get(), &CostComponent::componentChanged, this, &AbilityComponent::costReady);
    connect(qmlCost.get(), &CostComponent::close, this, &AbilityComponent::destroyCost);
}

void AbilityComponent::costReady(const std::optional<asn::Cost> &c) {
    cost = c;
    emit componentChanged(constructAbility());
}

void AbilityComponent::editEffects() {
    qmlEffects = std::make_unique<ArrayOfEffectsComponent>(effects, qmlObject);

    connect(qmlEffects.get(), &ArrayOfEffectsComponent::componentChanged, this, &AbilityComponent::effectsReady);
    connect(qmlEffects.get(), &ArrayOfEffectsComponent::close, this, &AbilityComponent::destroyEffects);
}

void AbilityComponent::effectsReady(const std::vector<asn::Effect> &e) {
    effects = e;
    emit componentChanged(constructAbility());
}

void AbilityComponent::componentReady() {
    emit componentChanged(constructAbility());
    emit close();
}

asn::Ability AbilityComponent::constructAbility() {
    asn::Ability ab;
    ab.type = type;
    setSpecificAbility(ab);
    return ab;
}

void AbilityComponent::setSpecificAbility(asn::Ability &a) {
    switch (type) {
    case asn::AbilityType::Auto:
        a.ability = constructSpecificAbility<asn::AutoAbility>();
        break;
    case asn::AbilityType::Cont:
        a.ability = constructSpecificAbility<asn::ContAbility>();
        break;
    case asn::AbilityType::Act:
        a.ability = constructSpecificAbility<asn::ActAbility>();
        break;
    case asn::AbilityType::Event:
        a.ability = constructSpecificAbility<asn::EventAbility>();
        break;
    }
}
