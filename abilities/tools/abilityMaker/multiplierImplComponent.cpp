#include "multiplierImplComponent.h"

#include <unordered_set>

#include <QQmlContext>


namespace {
const asn::Target& getTarget(MultiplierImplComponent::VarMultiplier &multiplier, asn::MultiplierType type) {
    switch (type) {
    case asn::MultiplierType::ForEach: {
        const auto &m = std::get<asn::ForEachMultiplier>(multiplier);
        return *m.target;
    }
    case asn::MultiplierType::AddLevel: {
        const auto &m = std::get<asn::AddLevelMultiplier>(multiplier);
        return *m.target;
    }
    case asn::MultiplierType::AddTriggerNumber: {
        const auto &m = std::get<asn::AddTriggerNumberMultiplier>(multiplier);
        return *m.target;
    }
    default:
        assert(false);
    }
    static auto t = asn::Target();
    return t;
}
} // namespace

MultiplierImplComponent::MultiplierImplComponent(asn::MultiplierType type,
                                                 const VarMultiplier &m,
                                                 QQuickItem *parent)
    : type(type), multiplier(m){
    init(parent);
}

MultiplierImplComponent::~MultiplierImplComponent() {
    if (qmlObject)
        qmlObject->deleteLater();
}

void MultiplierImplComponent::init(QQuickItem *parent) {
    std::unordered_map<asn::MultiplierType, QString> components {
        { asn::MultiplierType::ForEach, "ForEachMultiplier" },
        { asn::MultiplierType::AddLevel, "AddLevelMultiplier" },
        { asn::MultiplierType::AddTriggerNumber, "AddTriggerNumberMultiplier" }
    };
    std::unordered_set<asn::MultiplierType> readyComponents {
        asn::MultiplierType::TimesLevel
    };

    if (readyComponents.contains(type))
        return;

    if (!components.contains(type))
        return;

    QQmlComponent component(qmlEngine(parent), "qrc:/qml/basicTypes/" + components.at(type) + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setY(parent->property("multiplierImplY").toReal());

    switch (type) {
    case asn::MultiplierType::ForEach: {
        const auto &m = std::get<asn::ForEachMultiplier>(multiplier);
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, (int)m.placeType));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(placeTypeChanged(int)), this, SLOT(onPlaceTypeChanged(int)));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        break;
    }
    case asn::MultiplierType::AddLevel: {
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        break;
    }
    case asn::MultiplierType::AddTriggerNumber: {
        const auto &m = std::get<asn::AddTriggerNumberMultiplier>(multiplier);
        QMetaObject::invokeMethod(qmlObject, "setTriggerIcon", Q_ARG(QVariant, (int)m.triggerIcon));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(triggerIconChanged(int)), this, SLOT(onTriggerIconChanged(int)));
        break;
    }
    default:
        assert(false);
        break;
    }
}

void MultiplierImplComponent::editPlace() {
    const auto &place = *std::get<asn::ForEachMultiplier>(multiplier).place;
    qmlPlace = std::make_unique<PlaceComponent>(place, qmlObject);

    connect(qmlPlace.get(), &PlaceComponent::componentChanged, this, &MultiplierImplComponent::placeReady);
    connect(qmlPlace.get(), &PlaceComponent::close, this, &MultiplierImplComponent::destroyPlace);
}

void MultiplierImplComponent::destroyPlace() {
    qmlPlace.reset();
}

void MultiplierImplComponent::placeReady(const asn::Place &p) {
    switch (type) {
    case asn::MultiplierType::ForEach: {
        auto &m = std::get<asn::ForEachMultiplier>(multiplier);
        m.place = p;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(multiplier);
}

void MultiplierImplComponent::onPlaceTypeChanged(int value) {
    switch (type) {
    case asn::MultiplierType::ForEach: {
        auto &m = std::get<asn::ForEachMultiplier>(multiplier);
        m.placeType = static_cast<asn::PlaceType>(value);
        if (m.placeType != asn::PlaceType::SpecificPlace)
            m.place = std::nullopt;
        else {
            auto defaultPlace = asn::Place();
            defaultPlace.owner = asn::Player::Player;
            defaultPlace.pos = asn::Position::NotSpecified;
            defaultPlace.zone = asn::Zone::Stage;
            m.place = defaultPlace;
        }
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(multiplier);
}

void MultiplierImplComponent::onTriggerIconChanged(int value) {
    switch (type) {
    case asn::MultiplierType::AddTriggerNumber: {
        auto &m = std::get<asn::AddTriggerNumberMultiplier>(multiplier);
        m.triggerIcon = static_cast<asn::TriggerIcon>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(multiplier);
}

void MultiplierImplComponent::editTarget() {
    const auto &target = getTarget(multiplier, type);
    qmlTarget = std::make_unique<TargetComponent>(target, qmlObject);

    connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &MultiplierImplComponent::targetReady);
    connect(qmlTarget.get(), &TargetComponent::close, this, &MultiplierImplComponent::destroyTarget);
}

void MultiplierImplComponent::destroyTarget() {
    qmlTarget.reset();
}

void MultiplierImplComponent::targetReady(const asn::Target &t) {
    switch (type) {
    case asn::MultiplierType::ForEach: {
        auto &m = std::get<asn::ForEachMultiplier>(multiplier);
        m.target = std::make_shared<asn::Target>(t);
        break;
    }
    case asn::MultiplierType::AddLevel: {
        auto &m = std::get<asn::AddLevelMultiplier>(multiplier);
        m.target = std::make_shared<asn::Target>(t);
        break;
    }
    case asn::MultiplierType::AddTriggerNumber: {
        auto &m = std::get<asn::AddTriggerNumberMultiplier>(multiplier);
        m.target = std::make_shared<asn::Target>(t);
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(multiplier);
}
