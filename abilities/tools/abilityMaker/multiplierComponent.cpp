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
    case asn::MultiplierType::ForEach:
        needImpl = true;
        if (!multiplier.specifier) {
            multiplier.specifier = asn::ForEachMultiplier();
            multiplier.specifier->target = std::make_shared<asn::Target>();
            multiplier.specifier->placeType = asn::PlaceType::Selection;
        }
        break;
    }

    if (needImpl) {
        qmlMultiplierImpl = std::make_unique<MultiplierImplComponent>(multiplier, qmlObject);
    } else {
        qmlMultiplierImpl.reset();
        multiplier.specifier.reset();
    }

    emit componentChanged(multiplier);

    if (needImpl)
        connect(qmlMultiplierImpl.get(), &MultiplierImplComponent::componentChanged, this, &MultiplierComponent::onMultiplierChanged);
}

void MultiplierComponent::onMultiplierChanged(const asn::Multiplier &m) {
    multiplier = m;
    emit componentChanged(multiplier);
}
