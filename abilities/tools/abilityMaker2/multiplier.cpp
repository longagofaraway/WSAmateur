#include "multiplier.h"

#include <set>

#include "ability_maker_gen.h"
#include "language_parser.h"
#include "languageSpecification.h"
#include "multiplierInit.h"

namespace {
const qreal kLeftMargin = 10.0;
const qreal kTopMargin = 100.0;
}

MultiplierComponent::MultiplierComponent(QQuickItem *parent, QString id, QString displayName)
    : BaseComponent("Multiplier", parent, id) {
    gen_helper = std::make_shared<gen::MultiplierHelper>(this);
    qmlObject_->setProperty("displayName", displayName);
    connect(qmlObject_, SIGNAL(multiplierTypeChanged(QString)), this, SLOT(onMultiplierTypeChanged(QString)));
}

void MultiplierComponent::setMultiplier(asn::Multiplier) {

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
    asn::Multiplier m{.type=type_,.specifier=multiplier_.value()};
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

void MultiplierComponent::createMultiplier() {
    components_.clear();
    componentManager_.clear();

    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByEnum(QString::fromStdString(toString(type_)));
    std::set<std::string> types;
    for (const auto& comp: components) {
        QString component_id = comp.type + "/" + (types.contains(comp.type.toStdString()) ? QString("2") : QString(""));
        types.insert(comp.type.toStdString());
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject_, this, gen_helper.get());
        object->setProperty("visible", true);
        fitComponent(object);
    }

    //resizeMultiplier();
    qreal width{0};
    qreal maxHeight{0};
    for (const auto& component: components_) {
        width += component->width();
        maxHeight = std::max(maxHeight, component->height());
    }
    qmlObject_->setWidth(width + kLeftMargin*(components_.size()+1));
    qmlObject_->setHeight(maxHeight + kTopMargin + 2);

    gen_helper->setMultiplierInQml(type_, multiplier_.value());
    notifyOfChanges();
}
