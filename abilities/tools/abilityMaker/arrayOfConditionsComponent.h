#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"

class ConditionComponent;

class ArrayOfConditionsComponent : public BaseComponent
{
    Q_OBJECT
private:
    std::vector<QQuickItem*> qmlConditions;
    std::vector<asn::Condition> conditions;
    std::vector<bool> conditionsSet;

    std::shared_ptr<ConditionComponent> qmlCondition;

    int currentPos = 0;

public:
    ArrayOfConditionsComponent(QQuickItem *parent);
    ArrayOfConditionsComponent(const std::vector<asn::Condition> &ab, QQuickItem *parent);

signals:
    void componentChanged(const std::vector<asn::Condition> &a);

private slots:
    void addCondition();
    void editCondition(int pos);

    void destroyCondition();
    void conditionReady(const asn::Condition &condition);

private:
    void componentReady() override;
    void init();
    void createCondition();
};

