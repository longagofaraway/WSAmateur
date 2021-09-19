#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "effectComponent.h"

class CostComponent : public BaseComponent
{
    Q_OBJECT
private:
    using CostItem = std::pair<QQuickItem*, QQuickItem*>;
    std::vector<CostItem> qmlCostItems;
    std::vector<asn::CostItem> costItems;
    std::vector<bool> effectSet;
    bool initState = false;

    std::unique_ptr<EffectComponent> qmlEffect;

    int currentPos = 0;

public:
    CostComponent(QQuickItem *parent);
    CostComponent(const asn::Cost &cost, QQuickItem *parent);

signals:
    void componentChanged(const std::optional<asn::Cost> &card);
    void passCardSpecifierType(int index);

private slots:
    void addCost();
    void onCostTypeChanged(int pos, int value);
    void stringSet(int pos, QString value);

    void editEffect(int pos);
    void destroyEffect();
    void effectReady(const asn::Effect &effect);

private:
    void createCost();
    void init();
    void componentReady() override;
    std::optional<asn::Cost> constructCost();
    void initComponent(int pos, QQuickItem *obj);
};
