#include "place.h"

#include "language_parser.h"
#include "languageSpecification.h"

PlaceComponent::PlaceComponent(QQuickItem *parent, QString id, QString displayName)
    : BaseComponent("BasicTypes/Place", parent, id) {
    qmlObject_->setProperty("displayName", displayName);
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByType(QString::fromStdString("Place"));
    for (const auto& comp: components) {
        QString component_id = comp.type;
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject_, this, nullptr);
        fitComponent(object);
    }
}

void PlaceComponent::fitComponent(QQuickItem* object) {
    if (components_.empty()) {
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", qmlObject_->property("left"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", qmlObject_->property("top"));
    } else {
        auto *lastObject = components_.back();
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", lastObject->property("left"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("topMargin", QVariant(15));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", lastObject->property("bottom"));
    }
    components_.push_back(object);
}

void PlaceComponent::setPlace(asn::Place place) {
    place_ = place;
    emit setPosition(QString::fromStdString(toString(place_.pos)), "Position");
    emit setPlayer(QString::fromStdString(toString(place_.owner)), "Player");
    emit setZone(QString::fromStdString(toString(place_.zone)), "Zone");
}

void PlaceComponent::notifyOfChanges() {
    emit placeReady(place_, componentId_);
}

void PlaceComponent::positionChanged(QString position, QString id) {
    place_.pos = parse(position.toStdString(), formats::To<asn::Position>{});
    notifyOfChanges();
}

void PlaceComponent::zoneChanged(QString zone, QString id) {
    place_.zone = parse(zone.toStdString(), formats::To<asn::Zone>{});
    notifyOfChanges();
}

void PlaceComponent::playerChanged(QString player, QString id) {
    place_.owner = parse(player.toStdString(), formats::To<asn::Player>{});
    notifyOfChanges();
}
