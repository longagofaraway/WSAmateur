#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "costComponent.h"
#include "arrayOfEffectsComponent.h"
#include "triggerComponent.h"

class AbilityComponent : public BaseComponent
{
    Q_OBJECT
private:
    std::unique_ptr<TriggerComponent> qmlTrigger;
    bool triggerSet = false;
    asn::Trigger trigger;

    asn::AbilityType type;
    int activationTimes = 0;
    std::vector<asn::Keyword> keywords;

    std::unique_ptr<ArrayOfEffectsComponent> qmlEffects;
    std::vector<asn::Effect> effects;

    std::unique_ptr<CostComponent> qmlCost;
    std::optional<asn::Cost> cost;

public:
    AbilityComponent(QQuickItem *parent);
    AbilityComponent(const asn::Ability &a, QQuickItem *parent);

    void removeButtons();
    void fixEventAbility();

signals:
    void componentChanged(const asn::Ability &ability);

public slots:
    void setAbilityType(int type_);
    void setActivationTimes(int times);
    void setKeywords(QVariant keywordList);

    void editTrigger();
    void triggerReady(const asn::Trigger &t);
    void destroyTrigger();


    void editEffects();
    void effectsReady(const std::vector<asn::Effect> &e);
    void destroyEffects();

    void editCost();
    void costReady(const std::optional<asn::Cost> &c);
    void destroyCost();

private:
    void init();
    void componentReady() override;
    asn::Ability constructAbility();
    void setSpecificAbility(asn::Ability &a);

    template<typename T>
    T constructSpecificAbility();
};

template<typename T>
T AbilityComponent::constructSpecificAbility() {
    T ability;
    if constexpr (std::is_same_v<T, asn::AutoAbility>) {
        ability.activationTimes = activationTimes;
        ability.trigger = trigger;
        ability.cost = cost;
    }
    ability.keywords = keywords;
    if constexpr (std::is_same_v<T, asn::ActAbility>) {
        if (cost.has_value())
            ability.cost = *cost;
    }
    ability.effects = effects;
    return ability;
}

