#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

namespace gen {
    class TriggerHelper;
}

class TriggerComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarTrigger = decltype(asn::Trigger::trigger);

private:
    std::vector<QQuickItem*> components_;
    ComponentManager componentManager_;

    asn::TriggerType type_;
    VarTrigger trigger_;
    std::shared_ptr<gen::TriggerHelper> gen_helper;

public:
    TriggerComponent(QQuickItem *parent);
    TriggerComponent(QQuickItem *parent, const std::vector<asn::Trigger>& triggers);
    void init(QQuickItem *parent);
    ~TriggerComponent();

    void notifyOfChanges() override;
    asn::TriggerType getLanguageComponentType(formats::To<asn::TriggerType>) override { return type_; }
    VarTrigger& getLanguageComponent(formats::To<VarTrigger>) override { return trigger_; }

signals:
    void componentChanged(std::vector<asn::Trigger> trigger);

private:
    void fitComponent(QQuickItem* object);
    void setTriggerInQml();
    void createTrigger();
    std::vector<asn::Trigger> constructTrigger();

private slots:
    void onTriggerTypeChanged(QString);
};



