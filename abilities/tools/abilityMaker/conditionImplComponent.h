#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "cardComponent.h"
#include "placeComponent.h"
#include "targetComponent.h"

class ArrayOfConditionsComponent;

class ConditionImplComponent : public QObject
{
    Q_OBJECT
public:
    using VarCondition = decltype(asn::Condition::cond);

private:
    QQuickItem *qmlObject = nullptr;

    asn::ConditionType type;
    VarCondition condition;

    std::unique_ptr<TargetComponent> qmlTarget;
    asn::Target target;
    bool targetSet = false;

    std::unique_ptr<PlaceComponent> qmlPlace;
    std::unique_ptr<CardComponent> qmlCard;
    std::unique_ptr<ArrayOfConditionsComponent> qmlConditions;

public:
    ConditionImplComponent(asn::ConditionType type, QQuickItem *parent);
    ConditionImplComponent(asn::ConditionType type, const VarCondition &c, QQuickItem *parent);
    ~ConditionImplComponent();

signals:
    void componentChanged(const VarCondition &e);

private slots:
    void editTarget();
    void destroyTarget();
    void targetReady(const asn::Target &t);

    void editCard();
    void cardReady(const asn::Card &card_);
    void destroyCard();

    void editPlace();
    void destroyPlace();
    void placeReady(const asn::Place &p);

    void onPlayerChanged(int value);
    void onExcludingThisChanged(bool value);

    void onNumModifierChanged(int value);
    void onNumValueChanged(QString value);

    void editConditionsField();
    void editConditions(const std::vector<asn::Condition> &conditions);
    void destroyConditions();
    void conditionsReady(const std::vector<asn::Condition> &conditions);

private:
    void init(QQuickItem *parent);
};

void initConditionByType(ConditionImplComponent::VarCondition &condition, asn::ConditionType type);
