#include "placeComponent.h"

PlaceComponent::PlaceComponent(const asn::Place &p, QQuickItem *parent)
    : BaseComponent("basicTypes/Place", parent, "place"), place(p) {
    connect(this, SIGNAL(initPosition(int)), qmlObject, SIGNAL(incomingPosition(int)));
    connect(this, SIGNAL(initZone(int)), qmlObject, SIGNAL(incomingZone(int)));
    connect(this, SIGNAL(initOwner(int)), qmlObject, SIGNAL(incomingOwner(int)));

    init(p);

    connect(qmlObject, SIGNAL(positionChanged(int)), this, SLOT(onPositionChanged(int)));
    connect(qmlObject, SIGNAL(zoneChanged(int)), this, SLOT(onZoneChanged(int)));
    connect(qmlObject, SIGNAL(ownerChanged(int)), this, SLOT(onOwnerChanged(int)));

    // set y after setting the parent
    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void PlaceComponent::onPositionChanged(int value) {
    place.pos = static_cast<asn::Position>(value);
    emit componentChanged(place);
}

void PlaceComponent::onZoneChanged(int value) {
    place.zone = static_cast<asn::Zone>(value);
    emit componentChanged(place);
}

void PlaceComponent::onOwnerChanged(int value) {
    place.owner = static_cast<asn::Player>(value);
    emit componentChanged(place);
}

void PlaceComponent::componentReady() {
    emit componentChanged(place);
    emit close();
}

void PlaceComponent::init(const asn::Place &p) {
    emit initPosition(static_cast<int>(p.pos));
    emit initZone(static_cast<int>(p.zone));
    emit initOwner(static_cast<int>(p.owner));
}
