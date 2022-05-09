#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"

class MultiplierComponent;

class CardComponent : public BaseComponent
{
    Q_OBJECT
private:
    using CardSpecifier = std::pair<QQuickItem*, QQuickItem*>;
    std::vector<CardSpecifier> qmlSpecifiers;
    std::vector<asn::CardSpecifier> specifiers;
    bool initState = false;

    std::shared_ptr<MultiplierComponent> qmlMultiplier;
    int currentPos = 0;

public:
    explicit CardComponent(QQuickItem *parent);
    CardComponent(const asn::Card &card, QQuickItem *parent);

    void moveToTop();

signals:
    void componentChanged(const asn::Card &card);
    void passCardSpecifierType(int index);

private slots:
    void addSpecifier();
    void onSpecifierChanged(int pos, int value);
    void enumSet(int pos, int value);
    void stringSet(int pos, QString value);

    void editMultiplier(int pos);
    void destroyMultiplier();
    void multiplierReady(const asn::Multiplier &m);

private:
    void createSpecifier();
    void init();
    void componentReady() override;
    asn::Card constructCard();
    void initComponent(int pos, QQuickItem *obj);
};

