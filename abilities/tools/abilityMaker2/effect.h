#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

namespace gen {
    class TriggerHelper;
}

class EffectComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarEffect = decltype(asn::Effect::effect);

private:
    QString nodeId_;
    std::vector<QQuickItem*> components_;
    ComponentManager componentManager_;

    asn::EffectType type_;
    VarEffect effect_;
    std::shared_ptr<gen::TriggerHelper> gen_helper;

public:
    EffectComponent(QString nodeId, QQuickItem *parent);
    EffectComponent(QString nodeId, QQuickItem *parent, const asn::Effect& effect);
    void init(QQuickItem *parent);
    ~EffectComponent();

    virtual asn::EffectType getLanguageComponentType(formats::To<asn::EffectType>) override { return type_; }
    virtual VarEffect& getLanguageComponent(formats::To<VarEffect>) override { return effect_; }

signals:
    void componentChanged(QString nodeId, asn::EffectType type, VarEffect effect);

private:
    void fitComponent(QQuickItem* object);
    void createEffect();

private slots:
    void onEffectTypeChanged(QString);

};

