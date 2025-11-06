#include "effect.h"

#include <set>

#include "languageSpecification.h"
#include "language_parser.h"
#include "triggerInit.h"

EffectComponent::EffectComponent(QQuickItem *parent, QQuickItem *abilityMaker)
    : BaseComponent("Effect", parent) {
    init(parent, abilityMaker);
}

EffectComponent::EffectComponent(QQuickItem *parent, QQuickItem *abilityMaker, const asn::Effect& effect)
    : BaseComponent("Effect", parent) {
    init(parent, abilityMaker);
    type_ = effect.type;
    effect_ = effect.effect;
    QMetaObject::invokeMethod(qmlObject, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createEffect();
}

void EffectComponent::init(QQuickItem *parent, QQuickItem *abilityMaker) {
    abilityMaker_ = abilityMaker;
    qvariant_cast<QObject*>(qmlObject->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    connect(qmlObject, SIGNAL(effectTypeChanged(QString)), this, SLOT(onEffectTypeChanged(QString)));
}

EffectComponent::~EffectComponent() {
    /*for (auto *component: components_) {
        component->deleteLater();
    }*/
}

void EffectComponent::createEffect() {
    componentManager_.clear();
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponents(QString::fromStdString(toString(type_)));
    std::set<std::string> types;
    for (const auto& comp: components) {
        QString component_id = comp.type + (types.contains(comp.type.toStdString()) ? QString("2") : QString(""));
        types.insert(comp.type.toStdString());
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject, this);
        //fitComponent(object);
    }

    //setTriggerInQml();
}
