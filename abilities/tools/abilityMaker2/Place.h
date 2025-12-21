#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

class PlaceComponent : public BaseComponent
{
    Q_OBJECT
private:
    std::vector<QQuickItem*> components_;
    ComponentManager componentManager_;

    asn::Place place_;

public:
    PlaceComponent(QQuickItem *parent, QString id, QString displayName);

    void setPlace(asn::Place);
    void notifyOfChanges() override;

public slots:
    void positionChanged(QString position, QString id);
    void zoneChanged(QString zone, QString id);
    void playerChanged(QString player, QString id);

private:
    void fitComponent(QQuickItem* object);
};
