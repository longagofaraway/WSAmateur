#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "targetComponent.h"

class TriggerImplComponent : public QObject
{
    Q_OBJECT
public:
    using VarTrigger = decltype(asn::Trigger::trigger);

private:
    QQuickItem *qmlObject;

    asn::TriggerType type;
    VarTrigger trigger;

    std::unique_ptr<TargetComponent> qmlTarget;
    asn::Target target;
    bool targetSet = false;

    asn::PhaseTrigger phaseTrigger;

    std::unique_ptr<CardComponent> qmlCard;
    asn::Card card;
    bool cardSet = false;

public:
    TriggerImplComponent(asn::TriggerType type, QQuickItem *parent);
    TriggerImplComponent(asn::TriggerType type, const VarTrigger &tr, QQuickItem *parent);
    ~TriggerImplComponent();

signals:
    void componentChanged(VarTrigger trigger);

private slots:
    void componentReady();
    void setFrom(int index);
    void setTo(int index);
    void editTarget();
    void destroyTarget();
    void targetReady(const asn::Target &t);

    void setPhaseState(int index);
    void setPhase(int index);
    void setOwner(int index);

    void editCard();
    void clearCard();
    void cardReady(const asn::Card &card_);
    void destroyCard();

private:
    void init(QQuickItem *parent);
};

void initTriggerByType(TriggerImplComponent::VarTrigger &trigger, asn::TriggerType type);

