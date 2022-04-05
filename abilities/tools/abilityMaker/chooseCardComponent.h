#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "placeComponent.h"
#include "targetComponent.h"

class ChooseCardComponent : public BaseComponent
{
    Q_OBJECT
private:
    asn::ChooseCard chooseCard;

    std::unique_ptr<TargetComponent> qmlTarget;
    std::unique_ptr<PlaceComponent> qmlPlace;

public:
    ChooseCardComponent(const asn::ChooseCard &e, QQuickItem *parent);

signals:
    void componentChanged(const asn::ChooseCard &e);
    void initPlaceType(int value);
    void initExecutor(int value);

private slots:
    void onPlaceTypeChanged(int value);
    void onExecutorChanged(int value);

    void editTarget();
    void destroyTarget();
    void targetReady(const asn::Target &t);

    void editPlace();
    void destroyPlace();
    void placeReady(const asn::Place &p);

private:
    void componentReady() override;
    void init(const asn::ChooseCard &e);
};

