#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

class EffectComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarEffect = decltype(asn::Effect::effect);

private:
    QQuickItem *abilityMaker_;
    ComponentManager componentManager_;

    asn::EffectType type_;
    VarEffect effect_;

public:
    EffectComponent(QQuickItem *parent, QQuickItem *abilityMaker);
    EffectComponent(QQuickItem *parent, QQuickItem *abilityMaker, const asn::Effect& effect);
    void init(QQuickItem *parent, QQuickItem *abilityMaker);
    ~EffectComponent();

private:
    void createEffect();

};

