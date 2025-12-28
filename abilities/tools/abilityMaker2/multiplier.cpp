#include "multiplier.h"

#include <set>

#include "ability_maker_gen.h"
#include "language_parser.h"
#include "languageSpecification.h"
#include "multiplierInit.h"

namespace {
const qreal kLeftMargin = 10.0;
const qreal kTopMargin = 100.0;

VarMultiplier nullifyOptionalFields(asn::MultiplierType type, VarMultiplier multiplier) {
    switch (type) {
    case asn::MultiplierType::ForEach: {
        auto &elem = std::get<asn::ForEachMultiplier>(multiplier);
        if (elem.placeType != asn::PlaceType::SpecificPlace)
            elem.place = std::nullopt;
        if (elem.placeType != asn::PlaceType::Marker)
            elem.markerBearer = std::nullopt;
        break;
    }
    }
    return multiplier;
}
}

MultiplierComponent::MultiplierComponent(QQuickItem *parent, QString id, QString displayName)
    : BaseComponent("Multiplier", parent, id) {
    gen_helper = std::make_shared<gen::MultiplierHelper>(this);
    qmlObject_->setProperty("displayName", displayName);
    connect(qmlObject_, SIGNAL(multiplierTypeChanged(QString)), this, SLOT(onMultiplierTypeChanged(QString)));
}

void MultiplierComponent::setMultiplier(asn::Multiplier multiplier) {
    type_ = multiplier.type;
    multiplier_ = multiplier.specifier;
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createMultiplier();
}

void MultiplierComponent::fitComponent(QQuickItem* object) {
    if (components_.empty()) {
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", qmlObject_->property("left"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(kLeftMargin));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", qmlObject_->property("top"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("topMargin", QVariant(100));
    } else {
        auto *lastObject = components_.back();
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", lastObject->property("right"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(kLeftMargin));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", lastObject->property("top"));
    }
    components_.push_back(object);
}

void MultiplierComponent::notifyOfChanges() {
    asn::Multiplier m{.type=type_,.specifier=nullifyOptionalFields(type_, multiplier_.value())};
    emit multiplierReady(m, componentId_);
}

void MultiplierComponent::onValueTypeChanged(QString value,QString) {
    auto valueType = parse(value.toStdString(), formats::To<asn::ValueType>{});
    bool show = (valueType == asn::ValueType::Multiplier);
    if (show && !multiplier_) {
        QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
        multiplier_ = getDefaultMultiplier(type_);
        createMultiplier();
    }
    qmlObject_->setProperty("visible", show);
}

void MultiplierComponent::onMultiplierTypeChanged(QString value) {
    auto type = parse(value.toStdString(), formats::To<asn::MultiplierType>{});
    if (type == type_)
        return;
    type_ = type;
    multiplier_ = getDefaultMultiplier(type_);
    createMultiplier();
}

void MultiplierComponent::onPlaceTypeChanged(QString value, QString id) {
    // hide/show place
    emit placeTypeChanged(value, id);

    auto type = parse(value.toStdString(), formats::To<asn::PlaceType>{});
    if (!markerBearer)
        return;
    if (type == asn::PlaceType::Marker)
        markerBearer->setProperty("visible", true);
    else
        markerBearer->setProperty("visible", false);
    notifyOfChanges();
}

void MultiplierComponent::createMultiplier() {
    components_.clear();
    componentManager_.clear();
    markerBearer = nullptr;

    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByEnum(QString::fromStdString(toString(type_)));
    std::set<std::string> types;
    for (const auto& comp: components) {
        QString component_id = comp.type + "/" + (types.contains(comp.type.toStdString()) ? QString("2") : QString(""));
        types.insert(comp.type.toStdString());
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject_, this, gen_helper.get());
        if (comp.name == "MarkerBearer") {
            object->setProperty("visible", false);
            markerBearer = object;
        }
        fitComponent(object);
    }

    resizeMultiplier();
    gen_helper->setMultiplierInQml(type_, multiplier_.value());
    notifyOfChanges();
}

void MultiplierComponent::resizeMultiplier() {
    qreal width{0};
    qreal maxHeight{0};
    for (const auto& component: components_) {
        width += component->width();
        maxHeight = std::max(maxHeight, component->height());
    }
    qmlObject_->setWidth(std::max(width, qreal{200}) + kLeftMargin*(components_.size()+1));
    qmlObject_->setHeight(maxHeight + kTopMargin + 2);
}
