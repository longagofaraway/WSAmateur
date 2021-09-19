#include "conditionImplComponent.h"

#include <unordered_map>

#include <QQmlContext>
#include <QString>

void initConditionByType(ConditionImplComponent::VarCondition &condition, asn::ConditionType type) {
    auto defaultNum = asn::Number();
    defaultNum.mod = asn::NumModifier::ExactMatch;
    defaultNum.value = 1;

    auto defaultPlace = asn::Place();
    defaultPlace.owner = asn::Player::Player;
    defaultPlace.pos = asn::Position::NotSpecified;
    defaultPlace.zone = asn::Zone::Stage;

    switch (type) {
    case asn::ConditionType::IsCard: {
        auto c = asn::ConditionIsCard();
        c.neededCard.push_back(asn::Card());
        condition = c;
        break;
    }
    case asn::ConditionType::HaveCards: {
        auto c = asn::ConditionHaveCard();
        c.invert = false;
        c.who = asn::Player::Player;
        c.howMany = defaultNum;
        c.where = defaultPlace;
        c.excludingThis = false;
        condition = c;
        break;
    }
    default: break;
    }
}

namespace {
const asn::Place& getPlace(ConditionImplComponent::VarCondition &condition, asn::ConditionType type) {
    switch (type) {
    case asn::ConditionType::HaveCards: {
        const auto &c = std::get<asn::ConditionHaveCard>(condition);
        return c.where;
    }
    default:
        assert(false);
    }
    static auto p = asn::Place();
    return p;
}
const asn::Card& getCard(ConditionImplComponent::VarCondition &condition, asn::ConditionType type) {
    switch (type) {
    case asn::ConditionType::IsCard: {
        const auto &c = std::get<asn::ConditionIsCard>(condition);
        return c.neededCard[0];
    }
    case asn::ConditionType::HaveCards: {
        const auto &c = std::get<asn::ConditionHaveCard>(condition);
        return c.whichCards;
    }
    default:
        assert(false);
    }
    static auto p = asn::Card();
    return p;
}
}

ConditionImplComponent::ConditionImplComponent(asn::ConditionType type, QQuickItem *parent)
    : type(type) {
    init(parent);
}

ConditionImplComponent::ConditionImplComponent(asn::ConditionType type, const VarCondition &c, QQuickItem *parent)
    : type(type) {
    init(parent);

    condition = c;
    switch (type) {
    case asn::ConditionType::IsCard: {
        const auto &cond = std::get<asn::ConditionIsCard>(c);
        target = cond.target;
        targetSet = true;
        break;
    }
    case asn::ConditionType::HaveCards: {
        const auto &cond = std::get<asn::ConditionHaveCard>(c);
        QMetaObject::invokeMethod(qmlObject, "setExcludingThis", Q_ARG(QVariant, cond.excludingThis));
        QMetaObject::invokeMethod(qmlObject, "setOwner", Q_ARG(QVariant, (int)cond.who));
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)cond.howMany.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(cond.howMany.value)));
        break;
    }
    default:
        break;
    }
}

ConditionImplComponent::~ConditionImplComponent() {
    qmlObject->deleteLater();
}

void ConditionImplComponent::init(QQuickItem *parent) {
    std::unordered_map<asn::ConditionType, QString> components {
        { asn::ConditionType::IsCard, "IsCard" },
        { asn::ConditionType::HaveCards, "HaveCards" }
    };
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/conditions/" + components.at(type) + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setY(parent->property("conditionImplY").toReal());

    initConditionByType(condition, type);

    switch (type) {
    case asn::ConditionType::IsCard:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        break;
    case asn::ConditionType::HaveCards:
        QMetaObject::invokeMethod(qmlObject, "setOwner", Q_ARG(QVariant, (int)asn::Player::Player));
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)asn::NumModifier::ExactMatch));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(1)));

        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        connect(qmlObject, SIGNAL(ownerChanged(int)), this, SLOT(onPlayerChanged(int)));
        connect(qmlObject, SIGNAL(excludingThisChanged(bool)), this, SLOT(onExcludingThisChanged(bool)));
        break;
    default: break;
    }
}

void ConditionImplComponent::editTarget() {
    if (targetSet)
        qmlTarget = std::make_unique<TargetComponent>(target, qmlObject);
    else
        qmlTarget = std::make_unique<TargetComponent>(qmlObject);

    connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &ConditionImplComponent::targetReady);
    connect(qmlTarget.get(), &TargetComponent::close, this, &ConditionImplComponent::destroyTarget);
}

void ConditionImplComponent::destroyTarget() {
    qmlTarget.reset();
}

void ConditionImplComponent::targetReady(const asn::Target &t) {
    targetSet = true;
    target = t;

    switch (type) {
    case asn::ConditionType::IsCard: {
        auto &c = std::get<asn::ConditionIsCard>(condition);
        c.target = target;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(condition);
}

void ConditionImplComponent::editCard() {
    qmlCard = std::make_unique<CardComponent>(getCard(condition, type), qmlObject);
    qmlCard->moveToTop();

    connect(qmlCard.get(), &CardComponent::componentChanged, this, &ConditionImplComponent::cardReady);
    connect(qmlCard.get(), &CardComponent::close, this, &ConditionImplComponent::destroyCard);
}

void ConditionImplComponent::cardReady(const asn::Card &card_) {
    switch (type) {
    case asn::ConditionType::IsCard: {
        auto &c = std::get<asn::ConditionIsCard>(condition);
        c.neededCard[0] = card_;
        break;
    }
    case asn::ConditionType::HaveCards: {
        auto &c = std::get<asn::ConditionHaveCard>(condition);
        c.whichCards = card_;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(condition);
}

void ConditionImplComponent::destroyCard() {
    qmlCard.reset();
}

void ConditionImplComponent::editPlace() {
    qmlPlace = std::make_unique<PlaceComponent>(getPlace(condition, type), qmlObject);

    connect(qmlPlace.get(), &PlaceComponent::componentChanged, this, &ConditionImplComponent::placeReady);
    connect(qmlPlace.get(), &PlaceComponent::close, this, &ConditionImplComponent::destroyPlace);
}

void ConditionImplComponent::destroyPlace() {
    qmlPlace.reset();
}

void ConditionImplComponent::placeReady(const asn::Place &p) {
    switch (type) {
    case asn::ConditionType::HaveCards: {
        auto &c = std::get<asn::ConditionHaveCard>(condition);
        c.where = p;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(condition);
}

void ConditionImplComponent::onPlayerChanged(int value) {
    switch (type) {
    case asn::ConditionType::HaveCards: {
        auto &c = std::get<asn::ConditionHaveCard>(condition);
        c.who = static_cast<asn::Player>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(condition);
}

void ConditionImplComponent::onExcludingThisChanged(bool value) {
    switch (type) {
    case asn::ConditionType::HaveCards: {
        auto &c = std::get<asn::ConditionHaveCard>(condition);
        c.excludingThis = value;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(condition);
}

void ConditionImplComponent::onNumModifierChanged(int value) {
    switch (type) {
    case asn::ConditionType::HaveCards: {
        auto &c = std::get<asn::ConditionHaveCard>(condition);
        c.howMany.mod = static_cast<asn::NumModifier>(value);
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(condition);
}

void ConditionImplComponent::onNumValueChanged(QString value) {
    bool ok;
    int val = value.toInt(&ok);
    if (!ok)
        return;

    switch (type) {
    case asn::ConditionType::HaveCards: {
        auto &c = std::get<asn::ConditionHaveCard>(condition);
        c.howMany.value = val;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(condition);
}
