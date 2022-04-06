#include "conditionImplComponent.h"

#include <unordered_map>
#include <unordered_set>

#include <QQmlContext>
#include <QString>

#include "arrayOfConditionsComponent.h"

void initConditionByType(ConditionImplComponent::VarCondition &condition, asn::ConditionType type) {
    auto defaultNum = asn::Number();
    defaultNum.mod = asn::NumModifier::AtLeast;
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
    case asn::ConditionType::CardsLocation: {
        auto c = asn::ConditionCardsLocation();
        c.place = defaultPlace;
        condition = c;
        break;
    }
    case asn::ConditionType::CheckMilledCards: {
        auto c = asn::ConditionCheckMilledCards();
        c.number = defaultNum;
        condition = c;
        break;
    }
    case asn::ConditionType::SumOfLevels: {
        auto c = asn::ConditionSumOfLevels();
        c.equalOrMoreThan = 0;
        condition = c;
        break;
    }
    case asn::ConditionType::DuringTurn: {
        auto c = asn::ConditionDuringTurn();
        c.player = asn::Player::Player;
        condition = c;
        break;
    }
    case asn::ConditionType::RevealedCard: {
        auto c = asn::ConditionRevealCard();
        c.number = defaultNum;
        condition = c;
        break;
    }
    case asn::ConditionType::PlayersLevel: {
        auto c = asn::ConditionPlayersLevel();
        c.value = defaultNum;
        condition = c;
        break;
    }
    case asn::ConditionType::And: {
        condition = asn::ConditionAnd();
        break;
    }
    case asn::ConditionType::Or: {
        condition = asn::ConditionOr();
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
    case asn::ConditionType::CardsLocation: {
        const auto &c = std::get<asn::ConditionCardsLocation>(condition);
        return c.place;
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
    case asn::ConditionType::CheckMilledCards: {
        const auto &c = std::get<asn::ConditionCheckMilledCards>(condition);
        return c.card;
    }
    case asn::ConditionType::RevealedCard: {
        const auto &c = std::get<asn::ConditionRevealCard>(condition);
        return c.card;
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
    case asn::ConditionType::CardsLocation: {
        const auto &cond = std::get<asn::ConditionCardsLocation>(c);
        target = cond.target;
        targetSet = true;
        break;
    }
    case asn::ConditionType::CheckMilledCards: {
        const auto &cond = std::get<asn::ConditionCheckMilledCards>(c);
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)cond.number.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(cond.number.value)));
        break;
    }
    case asn::ConditionType::RevealedCard: {
        const auto &cond = std::get<asn::ConditionRevealCard>(c);
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)cond.number.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(cond.number.value)));
        break;
    }
    case asn::ConditionType::SumOfLevels: {
        const auto &cond = std::get<asn::ConditionSumOfLevels>(c);
        QMetaObject::invokeMethod(qmlObject, "setSum", Q_ARG(QVariant, QString::number(cond.equalOrMoreThan)));
        break;
    }
    case asn::ConditionType::DuringTurn: {
        const auto &cond = std::get<asn::ConditionDuringTurn>(c);
        QMetaObject::invokeMethod(qmlObject, "setOwner", Q_ARG(QVariant, (int)cond.player));
        break;
    }
    case asn::ConditionType::PlayersLevel: {
        const auto &cond = std::get<asn::ConditionPlayersLevel>(c);
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)cond.value.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(cond.value.value)));
        break;
    }
    default:
        break;
    }
}

ConditionImplComponent::~ConditionImplComponent() {
    if (qmlObject)
        qmlObject->deleteLater();
}

void ConditionImplComponent::init(QQuickItem *parent) {
    std::unordered_map<asn::ConditionType, QString> components {
        { asn::ConditionType::IsCard, "IsCard" },
        { asn::ConditionType::HaveCards, "HaveCards" },
        { asn::ConditionType::And, "AndOr" },
        { asn::ConditionType::Or, "AndOr" },
        { asn::ConditionType::CardsLocation, "CardsLocation" },
        { asn::ConditionType::CheckMilledCards, "CheckMilledCards" },
        { asn::ConditionType::RevealedCard, "CheckMilledCards" },
        { asn::ConditionType::SumOfLevels, "SumOfLevels" },
        { asn::ConditionType::DuringTurn, "DuringTurn" },
        { asn::ConditionType::PlayersLevel, "PlayersLevel" },
    };

    std::unordered_set<asn::ConditionType> readyComponents {
        asn::ConditionType::NoCondition,
        asn::ConditionType::InBattleWithThis
    };
    if (readyComponents.contains(type))
        return;

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
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)asn::NumModifier::AtLeast));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(1)));

        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        connect(qmlObject, SIGNAL(ownerChanged(int)), this, SLOT(onPlayerChanged(int)));
        connect(qmlObject, SIGNAL(excludingThisChanged(bool)), this, SLOT(onExcludingThisChanged(bool)));
        break;
    case asn::ConditionType::CardsLocation:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        break;
    case asn::ConditionType::CheckMilledCards:
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)asn::NumModifier::AtLeast));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(1)));

        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::ConditionType::RevealedCard:
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)asn::NumModifier::AtLeast));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(1)));

        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::ConditionType::SumOfLevels:
        QMetaObject::invokeMethod(qmlObject, "setSum", Q_ARG(QVariant, QString::number(0)));

        connect(qmlObject, SIGNAL(sumChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::ConditionType::DuringTurn:
        QMetaObject::invokeMethod(qmlObject, "setOwner", Q_ARG(QVariant, (int)asn::Player::Player));

        connect(qmlObject, SIGNAL(ownerChanged(int)), this, SLOT(onPlayerChanged(int)));
        break;
    case asn::ConditionType::PlayersLevel:
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)asn::NumModifier::AtLeast));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(1)));

        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::ConditionType::And:
        connect(qmlObject, SIGNAL(editConditions()), this, SLOT(editConditionsField()));
        break;
    case asn::ConditionType::Or:
        connect(qmlObject, SIGNAL(editConditions()), this, SLOT(editConditionsField()));
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
    case asn::ConditionType::CardsLocation: {
        auto &c = std::get<asn::ConditionCardsLocation>(condition);
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
    case asn::ConditionType::CheckMilledCards: {
        auto &c = std::get<asn::ConditionCheckMilledCards>(condition);
        c.card = card_;
        break;
    }
    case asn::ConditionType::RevealedCard: {
        auto &c = std::get<asn::ConditionRevealCard>(condition);
        c.card = card_;
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
    case asn::ConditionType::CardsLocation: {
        auto &c = std::get<asn::ConditionCardsLocation>(condition);
        c.place = p;
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
    case asn::ConditionType::DuringTurn: {
        auto &c = std::get<asn::ConditionDuringTurn>(condition);
        c.player = static_cast<asn::Player>(value);
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
    case asn::ConditionType::CheckMilledCards: {
        auto &c = std::get<asn::ConditionCheckMilledCards>(condition);
        c.number.mod = static_cast<asn::NumModifier>(value);
        break;
    }
    case asn::ConditionType::RevealedCard: {
        auto &c = std::get<asn::ConditionRevealCard>(condition);
        c.number.mod = static_cast<asn::NumModifier>(value);
        break;
    }
    case asn::ConditionType::PlayersLevel: {
        auto &c = std::get<asn::ConditionPlayersLevel>(condition);
        c.value.mod = static_cast<asn::NumModifier>(value);
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
    case asn::ConditionType::CheckMilledCards: {
        auto &c = std::get<asn::ConditionCheckMilledCards>(condition);
        c.number.value = val;
        break;
    }
    case asn::ConditionType::RevealedCard: {
        auto &c = std::get<asn::ConditionRevealCard>(condition);
        c.number.value = val;
        break;
    }
    case asn::ConditionType::SumOfLevels: {
        auto &c = std::get<asn::ConditionSumOfLevels>(condition);
        c.equalOrMoreThan = val;
        break;
    }
    case asn::ConditionType::PlayersLevel: {
        auto &c = std::get<asn::ConditionPlayersLevel>(condition);
        c.value.value = val;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(condition);
}

void ConditionImplComponent::editConditionsField() {
    std::vector<asn::Condition> *cond = nullptr;
    switch (type) {
    case asn::ConditionType::And:
        cond = &std::get<asn::ConditionAnd>(condition).cond;
        break;
    case asn::ConditionType::Or:
        cond = &std::get<asn::ConditionOr>(condition).cond;
        break;
    default: assert(false);
    }

    editConditions(*cond);
    connect(qmlConditions.get(), &ArrayOfConditionsComponent::componentChanged, this, &ConditionImplComponent::conditionsReady);
}

void ConditionImplComponent::editConditions(const std::vector<asn::Condition> &conditions) {
    qmlConditions = std::make_unique<ArrayOfConditionsComponent>(conditions, qmlObject);

    connect(qmlConditions.get(), &ArrayOfConditionsComponent::close, this, &ConditionImplComponent::destroyConditions);
}

void ConditionImplComponent::destroyConditions() {
    qmlConditions.reset();
}

void ConditionImplComponent::conditionsReady(const std::vector<asn::Condition> &conditions) {
    switch (type) {
    case asn::ConditionType::And: {
        auto &c = std::get<asn::ConditionAnd>(condition);
        c.cond = conditions;
        break;
    }
    case asn::ConditionType::Or: {
        auto &c = std::get<asn::ConditionOr>(condition);
        c.cond = conditions;
        break;
    }
    default: assert(false);
    }
    emit componentChanged(condition);
}
