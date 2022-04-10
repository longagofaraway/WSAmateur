#include "multiplierComponent.h"

MultiplierComponent::MultiplierComponent(QQuickItem *parent)
    : BaseComponent("basicTypes/Multiplier", parent, "multiplier") {
    init();
}

MultiplierComponent::MultiplierComponent(const asn::Multiplier &m, QQuickItem *parent)
    : BaseComponent("basicTypes/Multiplier", parent, "multiplier") {
    init();
    initMultiplier(m);
}

void MultiplierComponent::init() {
    connect(qmlObject, SIGNAL(multiplierTypeChanged(int)), this, SLOT(setMultiplierType(int)));

    connect(this, SIGNAL(passMultiplierType(int)), qmlObject, SIGNAL(incomingMultiplierType(int)));

    // set y after setting the parent
    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void MultiplierComponent::initMultiplier(const asn::Multiplier &m) {
    multiplier = m;
    emit passMultiplierType((int)multiplier.type);
}

void MultiplierComponent::componentReady() {
    emit componentChanged(multiplier);
    emit close();
}

void MultiplierComponent::setMultiplierType(int index) {
    bool needImpl = false;
    multiplier.type = static_cast<asn::MultiplierType>(index);
    switch (multiplier.type) {
    case asn::MultiplierType::ForEach: {
        needImpl = true;
        auto m = asn::ForEachMultiplier();
        m = asn::ForEachMultiplier();
        m.target = std::make_shared<asn::Target>();
        m.placeType = asn::PlaceType::Selection;
        multiplier.specifier = m;
        break;
    }
    case asn::MultiplierType::AddLevel: {
        needImpl = true;
        auto m = asn::AddLevelMultiplier();
        m.target = std::make_shared<asn::Target>();
        multiplier.specifier = m;
        break;
    }
    case asn::MultiplierType::AddTriggerNumber: {
        needImpl = true;
        auto m = asn::AddTriggerNumberMultiplier();
        m.target = std::make_shared<asn::Target>();
        m.triggerIcon = asn::TriggerIcon::Soul;
        multiplier.specifier = m;
        break;
    }
    }

    if (needImpl) {
        qmlMultiplierImpl = std::make_unique<MultiplierImplComponent>(multiplier.type, multiplier.specifier, qmlObject);
    } else {
        qmlMultiplierImpl.reset();
        multiplier.specifier = std::monostate();
    }

    emit componentChanged(multiplier);

    if (needImpl)
        connect(qmlMultiplierImpl.get(), &MultiplierImplComponent::componentChanged, this, &MultiplierComponent::onMultiplierChanged);
}

void MultiplierComponent::onMultiplierChanged(const MultiplierImplComponent::VarMultiplier &m) {
    multiplier.specifier = m;
    emit componentChanged(multiplier);
}
