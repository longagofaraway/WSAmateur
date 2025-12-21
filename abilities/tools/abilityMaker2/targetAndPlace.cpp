#include "targetAndPlace.h"

#include "language_parser.h"
#include "languageSpecification.h"

TargetAndPlaceComponent::TargetAndPlaceComponent(QQuickItem *parent, QString id)
    : BaseComponent("TargetAndPlace", parent, id) {
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByType(QString::fromStdString("TargetAndPlace"));
    for (const auto& comp: components) {
        QString component_id = comp.type;
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject_, this, nullptr);
        fitComponent(object);
    }
}

void TargetAndPlaceComponent::fitComponent(QQuickItem* object) {
    if (components_.empty()) {
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", qmlObject_->property("left"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", qmlObject_->property("top"));
    } else {
        auto *lastObject = components_.back();
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", lastObject->property("right"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(10));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", lastObject->property("top"));
    }
    components_.push_back(object);
}

void TargetAndPlaceComponent::setPlaceVisibility() {
    if (target_.placeType == asn::PlaceType::SpecificPlace) {
        components_.back()->setProperty("visible", true);
    } else {
        components_.back()->setProperty("visible", false);
    }
}

void TargetAndPlaceComponent::setTargetAndPlace(asn::TargetAndPlace target) {
    target_ = target;
    emit setTarget(target_.target, "Target");
    emit setPlaceType(QString::fromStdString(toString(target_.placeType)), "PlaceType");
    if (target.place)
        emit setPlace(target.place.value(), "Place");
    setPlaceVisibility();
}

void TargetAndPlaceComponent::notifyOfChanges() {
    emit targetAndPlaceReady(target_, componentId_);
}

void TargetAndPlaceComponent::targetChanged(const asn::Target &target, QString id) {
    target_.target = target;
    notifyOfChanges();
}

void TargetAndPlaceComponent::placeTypeChanged(QString placeType, QString id) {
    target_.placeType = parse(placeType.toStdString(), formats::To<asn::PlaceType>{});
    setPlaceVisibility();
    notifyOfChanges();
}

void TargetAndPlaceComponent::placeChanged(const asn::Place &place, QString id) {
    target_.place = place;
    notifyOfChanges();
}
