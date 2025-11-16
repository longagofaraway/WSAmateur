#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

class TriggerComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarTrigger = decltype(asn::Trigger::trigger);

private:
    QQuickItem *abilityMaker_;
    std::vector<QQuickItem*> components_;
    ComponentManager componentManager_;

    asn::TriggerType type_;
    VarTrigger trigger_;

public:
    TriggerComponent(QQuickItem *parent, QQuickItem *abilityMaker);
    TriggerComponent(QQuickItem *parent, QQuickItem *abilityMaker, const std::vector<asn::Trigger>& triggers);
    void init(QQuickItem *parent, QQuickItem *abilityMaker);
    ~TriggerComponent();

signals:
    void componentChanged(std::vector<asn::Trigger> trigger);

private:
    void fitComponent(QQuickItem* object);
    void setTriggerInQml();
    void createTrigger();
    std::vector<asn::Trigger> constructTrigger();

private slots:
    void onTriggerTypeChanged(QString);

    void zoneChanged(QString, QString);
    void targetChanged(const asn::Target& target);
    void phaseChanged(QString, QString);
    void phaseStateChanged(QString, QString);
    void playerChanged(QString, QString);
    void stateChanged(QString, QString);
    void attackTypeChanged(QString, QString);
    void boolChanged(bool, QString);
    void abilityTypeChanged(QString, QString);
};

