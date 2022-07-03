#include "effectComponent.h"

namespace {
asn::EffectType getEffectTypeFromComboBox(int index, QString text) {
    if (text == "Other Effect")
        return asn::EffectType::OtherEffect;
    return static_cast<asn::EffectType>(index);
}
}

EffectComponent::EffectComponent(QQuickItem *parent, int position)
    : BaseComponent("Effect", parent, "effect " + QString::number(position)) {
    init();
}

EffectComponent::EffectComponent(const asn::Effect &e, QQuickItem *parent, int position)
    : BaseComponent("Effect", parent, "effect " + QString::number(position)) {
    init();
    initEffect(e);
}

void EffectComponent::init() {
    connect(qmlObject, SIGNAL(effectTypeChanged(int,QString)), this, SLOT(setEffectType(int,QString)));

    connect(this, SIGNAL(passEffectType(int)), qmlObject, SIGNAL(incomingEffectType(int)));
}

void EffectComponent::initEffect(const asn::Effect &e) {
    initializing = true;
    type = e.type;
    effect = e.effect;
    emit passEffectType((int)type);
}

void EffectComponent::componentReady() {
    emit componentChanged(constructEffect());
    emit close();
}

asn::Effect EffectComponent::constructEffect() {
    asn::Effect e;
    e.type = type;
    e.effect = effect;
    return e;
}

void EffectComponent::setEffectType(int index, QString text) {
    bool needImpl = true;
    type = getEffectTypeFromComboBox(index, text);

    if (initializing) {
        if (needImpl)
            qmlEffectImpl = std::make_unique<EffectImplComponent>(type, effect, qmlObject);
        else
            qmlEffectImpl.reset();
        initializing = false;
        emit componentChanged(constructEffect());
    } else {
        if (needImpl) {
            qmlEffectImpl = std::make_unique<EffectImplComponent>(type, qmlObject);
            initEffectByType(effect, type);
        } else {
            qmlEffectImpl.reset();
        }
    }

    if (needImpl)
        connect(qmlEffectImpl.get(), &EffectImplComponent::componentChanged, this, &EffectComponent::onEffectChanged);
}

void EffectComponent::onEffectChanged(const VarEffect &e) {
    effect = e;
    emit componentChanged(constructEffect());
}
