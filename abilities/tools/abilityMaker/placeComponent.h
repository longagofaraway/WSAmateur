#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"

class PlaceComponent : public BaseComponent
{
    Q_OBJECT
private:
    asn::Place place;

public:
    PlaceComponent(const asn::Place &p, QQuickItem *parent);

signals:
    void componentChanged(const asn::Place &p);
    void initPosition(int value);
    void initZone(int value);
    void initOwner(int value);

private slots:
    void onPositionChanged(int value);
    void onZoneChanged(int value);
    void onOwnerChanged(int value);

private:
    void componentReady() override;
    void init(const asn::Place &p);
};

