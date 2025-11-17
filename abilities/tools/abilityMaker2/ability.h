#pragma once

#include <vector>

#include <QQuickItem>
#include <QString>

#include "abilities.h"
#include "baseComponent.h"

Q_DECLARE_METATYPE(asn::Ability);

class EffectsTree;

class AbilityComponent : public QQuickItem {
    Q_OBJECT
private:
    std::vector<asn::Trigger> triggers_;
    asn::AbilityType type_{asn::AbilityType::Auto};
    int activationTimes_ = 0;
    std::optional<asn::Cost> cost_;
    std::vector<asn::Keyword> keywords_;
    std::vector<asn::Effect> effects_;

    std::shared_ptr<BaseComponent> currentComponent_;

public:
    Q_INVOKABLE void createTrigger(QString triggerId, QQuickItem *parent);
    Q_INVOKABLE void openTrigger(QQuickItem *parent);

    void setCurrentComponent(std::shared_ptr<BaseComponent> component);
    void subscribeToEffectsChange(EffectsTree* effectsTree);

signals:
    void componentChanged(asn::Ability ability);
    void translationChanged(QString text);

private:
    asn::Ability constructAbility();
    template<typename T>
    T constructSpecificAbility();

private slots:
    void triggersChanged(const std::vector<asn::Trigger>& trigger);
    void effectsChanged(const std::vector<asn::Effect>& effects);

protected:
    void componentComplete() override;
};

template<typename T>
T AbilityComponent::constructSpecificAbility() {
T ability;
    if constexpr (std::is_same_v<T, asn::AutoAbility>) {
        ability.activationTimes = activationTimes_;
        ability.triggers = triggers_;
        ability.cost = cost_;
    }
    ability.keywords = keywords_;
    if constexpr (std::is_same_v<T, asn::ActAbility>) {
        if (cost_.has_value())
            ability.cost = cost_.value();
    }
    ability.effects = effects_;
    return ability;
}
