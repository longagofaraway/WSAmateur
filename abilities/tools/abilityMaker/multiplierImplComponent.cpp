#include "multiplierImplComponent.h"

#include <unordered_set>

#include <QQmlContext>


MultiplierImplComponent::MultiplierImplComponent(const asn::Multiplier &m,
                                                 QQuickItem *parent)
    : multiplier(m){
    init(parent);
}

MultiplierImplComponent::~MultiplierImplComponent() {
    if (qmlObject)
        qmlObject->deleteLater();
}

void MultiplierImplComponent::init(QQuickItem *parent) {
    std::unordered_map<asn::MultiplierType, QString> components {
        { asn::MultiplierType::ForEach, "ForEachMultiplier" }
    };
    std::unordered_set<asn::MultiplierType> readyComponents {
        asn::MultiplierType::TimesLevel,
        asn::MultiplierType::MarkersPutInWrThisWay
    };

    if (readyComponents.contains(multiplier.type))
        return;

    if (!components.contains(multiplier.type))
        return;

    QQmlComponent component(qmlEngine(parent), "qrc:/qml/basicTypes/" + components.at(multiplier.type) + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setY(parent->property("multiplierImplY").toReal());

    switch (multiplier.type) {
    case asn::MultiplierType::ForEach:
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, (int)multiplier.specifier->placeType));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(placeTypeChanged(int)), this, SLOT(onPlaceTypeChanged(int)));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        break;
    default:
        assert(false);
        break;
    }
}

void MultiplierImplComponent::editPlace() {
    qmlPlace = std::make_unique<PlaceComponent>(multiplier.specifier->place.value(), qmlObject);

    connect(qmlPlace.get(), &PlaceComponent::componentChanged, this, &MultiplierImplComponent::placeReady);
    connect(qmlPlace.get(), &PlaceComponent::close, this, &MultiplierImplComponent::destroyPlace);
}

void MultiplierImplComponent::destroyPlace() {
    qmlPlace.reset();
}

void MultiplierImplComponent::placeReady(const asn::Place &p) {
    switch (multiplier.type) {
    case asn::MultiplierType::ForEach: {
        multiplier.specifier->place = p;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(multiplier);
}

void MultiplierImplComponent::onPlaceTypeChanged(int value) {
    multiplier.specifier->placeType = static_cast<asn::PlaceType>(value);
    if (multiplier.specifier->placeType != asn::PlaceType::SpecificPlace)
        multiplier.specifier->place = std::nullopt;
    else {
        auto defaultPlace = asn::Place();
        defaultPlace.owner = asn::Player::Player;
        defaultPlace.pos = asn::Position::NotSpecified;
        defaultPlace.zone = asn::Zone::Stage;
        multiplier.specifier->place = defaultPlace;
    }
}

void MultiplierImplComponent::editTarget() {
    qmlTarget = std::make_unique<TargetComponent>(*multiplier.specifier->target, qmlObject);

    connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &MultiplierImplComponent::targetReady);
    connect(qmlTarget.get(), &TargetComponent::close, this, &MultiplierImplComponent::destroyTarget);
}

void MultiplierImplComponent::destroyTarget() {
    qmlTarget.reset();
}

void MultiplierImplComponent::targetReady(const asn::Target &t) {
    multiplier.specifier->target = std::make_shared<asn::Target>(t);
    emit componentChanged(multiplier);
}
