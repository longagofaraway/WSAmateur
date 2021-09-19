#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "effectComponent.h"
#include "conditionComponent.h"

class ArrayOfEffectsComponent : public BaseComponent
{
    Q_OBJECT
private:
    using VarEffect = decltype(asn::Effect::effect);

    std::vector<QQuickItem*> qmlEffects;
    std::vector<asn::Effect> effects;
    std::vector<bool> effectSet;
    std::vector<bool> conditionSet;

    std::unique_ptr<EffectComponent> qmlEffect;
    std::unique_ptr<ConditionComponent> qmlCondition;

    int currentPos = 0;

public:
    ArrayOfEffectsComponent(QQuickItem *parent);
    ArrayOfEffectsComponent(const std::vector<asn::Effect> &ef, QQuickItem *parent);

signals:
    void componentChanged(const std::vector<asn::Effect> &e);

private slots:
    void addEffect();
    void editEffect(int pos);
    void editCondition(int pos);

    void destroyEffect();
    void effectReady(const asn::Effect &effect);

    void destroyCondition();
    void conditionReady(const asn::Condition &condition);

private:
    void componentReady() override;
    void init();
    void createEffect();
};

