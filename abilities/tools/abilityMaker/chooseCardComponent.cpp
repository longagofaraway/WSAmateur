#include "chooseCardComponent.h"

ChooseCardComponent::ChooseCardComponent(const asn::ChooseCard &e, QQuickItem *parent)
    : BaseComponent("effects/ChooseCard", parent), chooseCard(e) {
    connect(this, SIGNAL(initPlaceType(int)), qmlObject, SIGNAL(incomingPlaceType(int)));
    connect(this, SIGNAL(initExecutor(int)), qmlObject, SIGNAL(incomingExecutor(int)));

    init(e);

    connect(qmlObject, SIGNAL(placeTypeChanged(int)), this, SLOT(onPlaceTypeChanged(int)));
    connect(qmlObject, SIGNAL(executorChanged(int)), this, SLOT(onExecutorChanged(int)));
    connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
    connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));

    // set y after setting the parent
    QMetaObject::invokeMethod(qmlObject, "setActualY");
    QMetaObject::invokeMethod(qmlObject, "enableButtons");
}

void ChooseCardComponent::onPlaceTypeChanged(int value) {
    chooseCard.targets[0].placeType = static_cast<asn::PlaceType>(value);
    if (chooseCard.targets[0].placeType != asn::PlaceType::SpecificPlace)
        chooseCard.targets[0].place.reset();
    emit componentChanged(chooseCard);
}

void ChooseCardComponent::onExecutorChanged(int value) {
    chooseCard.executor = static_cast<asn::Player>(value);
    emit componentChanged(chooseCard);
}

void ChooseCardComponent::editTarget() {
    qmlTarget = std::make_unique<TargetComponent>(chooseCard.targets[0].target, qmlObject);

    connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &ChooseCardComponent::targetReady);
    connect(qmlTarget.get(), &TargetComponent::close, this, &ChooseCardComponent::destroyTarget);
}

void ChooseCardComponent::componentReady() {
    emit componentChanged(chooseCard);
    emit close();
}

void ChooseCardComponent::init(const asn::ChooseCard &e) {
    emit initPlaceType(static_cast<int>(e.targets[0].placeType));
    emit initExecutor(static_cast<int>(e.executor));
}


void ChooseCardComponent::destroyTarget() {
    qmlTarget.reset();
}

void ChooseCardComponent::targetReady(const asn::Target &t) {
    chooseCard.targets[0].target = t;
    emit componentChanged(chooseCard);
}

void ChooseCardComponent::editPlace() {
    const auto &optPlace = chooseCard.targets[0].place;
    asn::Place place;
    if (optPlace)
        place = *optPlace;
    else {
        place.owner = asn::Player::Player;
        place.pos = asn::Position::NotSpecified;
        place.zone = asn::Zone::Stage;
    }

    qmlPlace = std::make_unique<PlaceComponent>(place, qmlObject);

    connect(qmlPlace.get(), &PlaceComponent::componentChanged, this, &ChooseCardComponent::placeReady);
    connect(qmlPlace.get(), &PlaceComponent::close, this, &ChooseCardComponent::destroyPlace);
}

void ChooseCardComponent::destroyPlace() {
    qmlPlace.reset();
}

void ChooseCardComponent::placeReady(const asn::Place &p) {
    chooseCard.targets[0].place = p;
    emit componentChanged(chooseCard);
}

