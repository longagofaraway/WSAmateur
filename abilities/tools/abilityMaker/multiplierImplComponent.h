#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "cardComponent.h"
#include "placeComponent.h"
#include "targetComponent.h"

class MultiplierImplComponent : public QObject
{
    Q_OBJECT
private:
    QQuickItem *qmlObject = nullptr;

    asn::Multiplier multiplier;

    std::unique_ptr<TargetComponent> qmlTarget;
    std::unique_ptr<PlaceComponent> qmlPlace;

public:
    MultiplierImplComponent(const asn::Multiplier &m, QQuickItem *parent);
    ~MultiplierImplComponent();

signals:
    void componentChanged(const asn::Multiplier &m);

private slots:
    void editTarget();
    void destroyTarget();
    void targetReady(const asn::Target &t);

    void editPlace();
    void destroyPlace();
    void placeReady(const asn::Place &p);

    void onPlaceTypeChanged(int value);

private:
    void init(QQuickItem *parent);
};
