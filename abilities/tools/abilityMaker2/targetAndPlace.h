#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

class TargetAndPlaceComponent : public BaseComponent
{
    Q_OBJECT
private:
    std::vector<QQuickItem*> components_;
    ComponentManager componentManager_;

    asn::TargetAndPlace target_;

public:
    TargetAndPlaceComponent(QQuickItem *parent, QString id);

    void setTargetAndPlace(asn::TargetAndPlace);
    void notifyOfChanges() override;

public slots:
    void targetChanged(const asn::Target& target, QString id);
    void placeTypeChanged(QString placeType, QString id);
    void placeChanged(const asn::Place& place, QString id);

private:
    void fitComponent(QQuickItem* object);
    void setPlaceVisibility();
};
