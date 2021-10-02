#pragma once

#include <QObject>
#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "triggerImplComponent.h"

class TriggerComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarTrigger = decltype(asn::Trigger::trigger);

private:
    asn::TriggerType type;
    VarTrigger trigger;
    bool initializing = false;

    std::unique_ptr<TriggerImplComponent> qmlTriggerImpl;

public:
    TriggerComponent(QQuickItem *parent);
    TriggerComponent(const asn::Trigger &trigger, QQuickItem *parent);
    ~TriggerComponent();

signals:
    void componentChanged(const asn::Trigger &trigger);
    void passTriggerType(int type);

private slots:
    void setTriggerType(int index);
    void updateTrigger(int index);
    void onTriggerChanged(const VarTrigger &t);

private:
    void componentReady() override;
    void init(QQuickItem *parent);
    void initTrigger(const asn::Trigger &trigger);
    asn::Trigger constructTrigger();

    void fromHandToStagePreset();
    void thisCardAttacks();
    void thisCardBecomesReversed();
};
