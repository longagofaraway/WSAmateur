#include "effect.h"

#include <set>

#include "ability_maker_gen.h"
#include "languageSpecification.h"
#include "language_parser.h"
#include "effectInit.h"

EffectComponent::EffectComponent(QString nodeId, QQuickItem *parent)
    : BaseComponent("Effect", parent), nodeId_(nodeId) {
    init(parent);
}

EffectComponent::EffectComponent(QString nodeId, QQuickItem *parent, const asn::Effect& effect)
    : BaseComponent("Effect", parent), nodeId_(nodeId) {
    init(parent);
    type_ = effect.type;
    effect_ = effect.effect;
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createEffect();
}

void EffectComponent::init(QQuickItem *parent) {
    gen_helper = std::make_shared<gen::EffectHelper>(this);
    qvariant_cast<QObject*>(qmlObject_->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    connect(qmlObject_, SIGNAL(effectTypeChanged(QString)), this, SLOT(onEffectTypeChanged(QString)));
}

EffectComponent::~EffectComponent() {}

void EffectComponent::fitComponent(QQuickItem* object) {
    if (components_.empty()) {
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", qmlObject_->property("left"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(50));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", qmlObject_->property("top"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("topMargin", QVariant(130));
    } else {
        auto *lastObject = components_.back();
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", lastObject->property("right"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(10));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", lastObject->property("top"));
    }
    components_.push_back(object);
}

void EffectComponent::createEffect() {
    components_.clear();
    componentManager_.clear();
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByEnum(QString::fromStdString(toString(type_)));
    std::set<std::string> types;
    for (const auto& comp: components) {
        if (comp.type == "Effect") {
            continue;
        }
        QString component_id = comp.type + "/" + (types.contains(comp.type.toStdString()) ? QString("2") : QString(""));
        types.insert(comp.type.toStdString());
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject_, this, gen_helper.get());
        fitComponent(object);
    }

    gen_helper->setEffectInQml(type_, effect_);
}

void EffectComponent::onEffectTypeChanged(QString type) {
    qDebug() << "changing effect type";
    type_ = parse(type.toStdString(), formats::To<asn::EffectType>{});
    effect_ = getDefaultEffect(type_);
    createEffect();
}

void EffectComponent::notifyOfChanges() {
    emit componentChanged(nodeId_, type_, effect_);
    qreal width{70}, height{0};
    for (const auto component: components_) {
        width += component->width() + 10;
        height = std::max(height, component->height());
    }
    emit sizeChanged(width, height);
}
