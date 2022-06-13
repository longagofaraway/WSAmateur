#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "triggerComponent.h"

class ArrayOfTriggersComponent : public BaseComponent
{
    Q_OBJECT
private:
    using VarTrigger = decltype(asn::Trigger::trigger);

    asn::Trigger defaultTrigger;

    std::vector<QQuickItem*> qmlTriggers;
    std::vector<asn::Trigger> triggers;

    std::unique_ptr<TriggerComponent> qmlTrigger;

    int currentPos = 0;

public:
    ArrayOfTriggersComponent(QQuickItem *parent);
    ArrayOfTriggersComponent(const std::vector<asn::Trigger> &ef, QQuickItem *parent);

signals:
    void componentChanged(const std::vector<asn::Trigger> &t);

private slots:
    void addTrigger();
    void editTrigger(int pos);

    void destroyTrigger();
    void triggerReady(const asn::Trigger &trigger);

private:
    void componentReady() override;
    void init();
    void createTrigger();
};

