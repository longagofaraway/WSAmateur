#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "costComponent.h"
#include "dbControls.h"
#include "arrayOfEffectsComponent.h"
#include "arrayOfTriggersComponent.h"

class AbilityMaker;

class AbilityComponent : public BaseComponent
{
    Q_OBJECT
private:
    std::unique_ptr<ArrayOfTriggersComponent> qmlTriggers;
    std::vector<asn::Trigger> triggers;

    asn::AbilityType type;
    int activationTimes = 0;
    std::vector<asn::Keyword> keywords;

    std::unique_ptr<ArrayOfEffectsComponent> qmlEffects;
    std::vector<asn::Effect> effects;

    std::unique_ptr<CostComponent> qmlCost;
    std::optional<asn::Cost> cost;

    std::unique_ptr<DbControls> dbControls;

public:
    AbilityComponent(QQuickItem *parent, int position);
    AbilityComponent(const asn::Ability &a, QQuickItem *parent, int position);

    void removeButtons();
    void fixEventAbility();
    void fixAutoAbility();
    void addDbControls(AbilityMaker *maker);
    void setAbility(const asn::Ability &a);

signals:
    void componentChanged(const asn::Ability &ability);

public slots:
    void setAbilityType(int type_);
    void setActivationTimes(int times);
    void setKeywords(QVariant keywordList);

    void editTriggers();
    void triggersReady(const std::vector<asn::Trigger> &t);
    void destroyTriggers();

    void editEffects();
    void effectsReady(const std::vector<asn::Effect> &e);
    void destroyEffects();

    void editCost();
    void costReady(const std::optional<asn::Cost> &c);
    void destroyCost();

private:
    void init();
    void initAbility(const asn::Ability &a);
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
        ability.triggers = triggers;
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

