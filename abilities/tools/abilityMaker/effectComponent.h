#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "effectImplComponent.h"

class EffectComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarEffect = decltype(asn::Effect::effect);

private:
    asn::EffectType type;
    VarEffect effect;
    bool initializing = false;

    std::unique_ptr<EffectImplComponent> qmlEffectImpl;

public:
    EffectComponent(QQuickItem *parent);
    EffectComponent(const asn::Effect &e, QQuickItem *parent);

signals:
    void componentChanged(const asn::Effect &effect);
    void passEffectType(int type);

private slots:
    void setEffectType(int index);
    void onEffectChanged(const VarEffect &e);

private:
    void init();
    void initEffect(const asn::Effect &e);
    void componentReady() override;
    asn::Effect constructEffect();
};

