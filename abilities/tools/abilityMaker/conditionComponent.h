#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "conditionImplComponent.h"

class ConditionComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarCondition = decltype(asn::Condition::cond);

private:
    asn::ConditionType type;
    VarCondition condition;
    bool initializing = false;

    std::unique_ptr<ConditionImplComponent> qmlConditionImpl;

public:
    ConditionComponent(QQuickItem *parent, int position);
    ConditionComponent(const asn::Condition &c, QQuickItem *parent, int position);

signals:
    void componentChanged(const asn::Condition &condition);
    void passConditionType(int type);

private slots:
    void onConditionTypeChanged(int index);
    void onConditionChanged(const VarCondition &e);

private:
    void init();
    void initCondition(const asn::Condition &e);
    void componentReady() override;
    asn::Condition constructCondition();
};

