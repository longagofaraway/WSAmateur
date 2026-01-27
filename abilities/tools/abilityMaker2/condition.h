#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

namespace gen {
    class ConditionHelper;
}

class ConditionComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarCondition = decltype(asn::Condition::cond);

private:
    std::vector<QQuickItem*> components_;
    ComponentManager componentManager_;

    asn::ConditionType type_;
    VarCondition condition_;
    std::shared_ptr<gen::ConditionHelper> gen_helper;

public:
    ConditionComponent(QString nodeId, QQuickItem *parent);
    ConditionComponent(QString nodeId, QQuickItem *parent, const asn::Condition& condition);
    void init(QQuickItem *parent);
    ~ConditionComponent() = default;

    void notifyOfChanges() override;
    virtual asn::ConditionType getLanguageComponentType(formats::To<asn::ConditionType>) override { return type_; }
    virtual VarCondition& getLanguageComponent(formats::To<VarCondition>) override { return condition_; }

signals:
    void componentChanged(QString nodeId, asn::ConditionType type, VarCondition condition);
    void sizeChanged(qreal width, qreal height);

private:
    void fitComponent(QQuickItem* object);
    void createCondition();

private slots:
    void onConditionTypeChanged(QString);
};
