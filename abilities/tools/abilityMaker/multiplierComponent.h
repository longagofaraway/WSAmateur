#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "multiplierImplComponent.h"

class MultiplierComponent : public BaseComponent
{
    Q_OBJECT
private:
    asn::Multiplier multiplier;
    bool initializing = false;

    std::unique_ptr<MultiplierImplComponent> qmlMultiplierImpl;

public:
    MultiplierComponent(QQuickItem *parent);
    MultiplierComponent(const asn::Multiplier &m, QQuickItem *parent);

signals:
    void componentChanged(const asn::Multiplier &multiplier);
    void passMultiplierType(int type);

private slots:
    void setMultiplierType(int index);
    void onMultiplierChanged(const asn::Multiplier &m);

private:
    void init();
    void initMultiplier(const asn::Multiplier &m);
    void componentReady() override;
};

