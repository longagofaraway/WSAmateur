#include "effectImplComponent.h"

#include <unordered_map>
#include <unordered_set>

#include <QQmlContext>
#include <QString>

#include "arrayOfEffectsComponent.h"
#include "arrayOfAbilitiesComponent.h"

void initEffectByType(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    auto defaultNum = asn::Number();
    defaultNum.mod = asn::NumModifier::ExactMatch;
    defaultNum.value = 1;

    auto defaultPlace = asn::Place();
    defaultPlace.owner = asn::Player::Player;
    defaultPlace.pos = asn::Position::NotSpecified;
    defaultPlace.zone = asn::Zone::Stage;

    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto e = asn::AttributeGain();
        e.type = asn::AttributeType::Power;
        e.gainType = asn::ValueType::Raw;
        e.value = 0;
        e.duration = 1;
        effect = e;
        break;
    }
    case asn::EffectType::ChooseCard: {
        auto e = asn::ChooseCard();
        auto tp = asn::TargetAndPlace();
        tp.placeType = asn::PlaceType::SpecificPlace;
        tp.place = defaultPlace;
        tp.target = asn::Target();
        e.executor = asn::Player::Player;
        e.targets.push_back(tp);
        effect = e;
        break;
    }
    case asn::EffectType::RevealCard: {
        auto e = asn::RevealCard();
        e.number = defaultNum;
        e.type = asn::RevealType::TopDeck;
        effect = e;
        break;
    }
    case asn::EffectType::Look: {
        auto e = asn::Look();
        e.number = defaultNum;
        e.place = defaultPlace;
        effect = e;
        break;
    }
    case asn::EffectType::MoveCard: {
        auto e = asn::MoveCard();
        e.executor = asn::Player::Player;
        e.from = defaultPlace;
        e.to.push_back(defaultPlace);
        e.order = asn::Order::NotSpecified;
        effect = e;
        break;
    }
    case asn::EffectType::SearchCard: {
        auto e = asn::SearchCard();
        e.place = defaultPlace;
        auto st = asn::SearchTarget();
        st.number = defaultNum;
        st.cards.push_back(asn::Card());
        e.targets.push_back(st);
        effect = e;
        break;
    }
    case asn::EffectType::PayCost: {
        effect = asn::PayCost();
        break;
    }
    case asn::EffectType::AbilityGain: {
        auto e = asn::AbilityGain();
        e.duration = 1;
        e.number = 1;
        effect = e;
        break;
    }
    case asn::EffectType::PerformEffect: {
        auto e = asn::PerformEffect();
        e.numberOfEffects = 1;
        e.numberOfTimes = 1;
        effect = e;
        break;
    }
    case asn::EffectType::NonMandatory: {
        effect = asn::NonMandatory();
        break;
    }
    case asn::EffectType::Shuffle: {
        auto e  = asn::Shuffle();
        e.owner = asn::Player::Player;
        e.zone = asn::Zone::Deck;
        effect = e;
        break;
    }
    case asn::EffectType::DealDamage: {
        auto e = asn::DealDamage();
        e.damageType = asn::ValueType::Raw;
        e.damage = 1;
        effect = e;
        break;
    }
    case asn::EffectType::ChangeState: {
        auto e = asn::ChangeState();
        e.target = asn::Target();
        e.state = asn::State::Rested;
        effect = e;
        break;
    }
    case asn::EffectType::Backup: {
        auto e = asn::Backup();
        e.level = 1;
        e.power = 1000;
        effect = e;
        break;
    }
    case asn::EffectType::FlipOver: {
        auto e = asn::FlipOver();
        e.number = defaultNum;
        e.number.value = 4;
        effect = e;
        break;
    }
    case asn::EffectType::DrawCard: {
        auto e = asn::DrawCard();
        e.executor = asn::Player::Player;
        e.value.mod = asn::NumModifier::ExactMatch;
        e.value.value = 1;
        effect = e;
        break;
    }
    case asn::EffectType::TriggerCheckTwice:
    case asn::EffectType::EarlyPlay:
    case asn::EffectType::StockSwap:
        effect = std::monostate();
        break;
    default:
        break;
    }
}

namespace {
const asn::Place& getPlace(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        const auto &e = std::get<asn::ChooseCard>(effect);
        return *e.targets[0].place;
    }
    case asn::EffectType::Look: {
        const auto &e = std::get<asn::Look>(effect);
        return e.place;
    }
    case asn::EffectType::MoveCard: {
        const auto &e = std::get<asn::MoveCard>(effect);
        return e.from;
    }
    case asn::EffectType::SearchCard: {
        const auto &e = std::get<asn::SearchCard>(effect);
        return e.place;
    }
    default:
        assert(false);
    }
    static auto p = asn::Place();
    return p;
}
const asn::Card& getCard(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    switch (type) {
    case asn::EffectType::RevealCard: {
        const auto &e = std::get<asn::RevealCard>(effect);
        return *e.card;
    }
    case asn::EffectType::SearchCard: {
        const auto &e = std::get<asn::SearchCard>(effect);
        return e.targets[0].cards[0];
    }
    case asn::EffectType::FlipOver: {
        const auto &e = std::get<asn::FlipOver>(effect);
        return e.forEach;
    }
    default:
        assert(false);
    }
    static auto p = asn::Card();
    return p;
}
}

EffectImplComponent::EffectImplComponent(asn::EffectType type, QQuickItem *parent)
    : type(type) {
    init(parent);
}

EffectImplComponent::EffectImplComponent(asn::EffectType type, const VarEffect &e, QQuickItem *parent)
    : type(type) {
    init(parent);

    effect = e;
    switch (type) {
    case asn::EffectType::AttributeGain: {
        const auto &ef = std::get<asn::AttributeGain>(e);
        target = ef.target;
        targetSet = true;
        QMetaObject::invokeMethod(qmlObject, "setAttrType", Q_ARG(QVariant, (int)ef.type));
        QMetaObject::invokeMethod(qmlObject, "setValueInput", Q_ARG(QVariant, QString::number(ef.value)));
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, ef.duration));
        break;
    }
    case asn::EffectType::ChooseCard: {
        const auto &ef = std::get<asn::ChooseCard>(e);
        target = ef.targets[0].target;
        targetSet = true;
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, (int)ef.targets[0].placeType));
        QMetaObject::invokeMethod(qmlObject, "setExecutor", Q_ARG(QVariant, (int)ef.executor));
        break;
    }
    case asn::EffectType::MoveCard: {
        const auto &ef = std::get<asn::MoveCard>(e);
        target = ef.target;
        targetSet = true;
        QMetaObject::invokeMethod(qmlObject, "setOrder", Q_ARG(QVariant, (int)ef.order));
        QMetaObject::invokeMethod(qmlObject, "setExecutor", Q_ARG(QVariant, (int)ef.executor));
        break;
    }
    case asn::EffectType::Look: {
        const auto &ef = std::get<asn::Look>(e);
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)ef.number.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(ef.number.value)));
        break;
    }
    case asn::EffectType::SearchCard: {
        const auto &ef = std::get<asn::SearchCard>(e);
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)ef.targets[0].number.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(ef.targets[0].number.value)));
        break;
    }
    case asn::EffectType::AbilityGain: {
        const auto &ef = std::get<asn::AbilityGain>(e);
        target = ef.target;
        targetSet = true;
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        QMetaObject::invokeMethod(qmlObject, "setNumber", Q_ARG(QVariant, QString::number(ef.number)));
        break;
    }
    case asn::EffectType::PerformEffect: {
        const auto &ef = std::get<asn::PerformEffect>(e);
        QMetaObject::invokeMethod(qmlObject, "setEffectNum", Q_ARG(QVariant, QString::number(ef.numberOfEffects)));
        QMetaObject::invokeMethod(qmlObject, "setEffectTimes", Q_ARG(QVariant, QString::number(ef.numberOfTimes)));
        break;
    }
    case asn::EffectType::DealDamage: {
        const auto &ef = std::get<asn::DealDamage>(e);
        QMetaObject::invokeMethod(qmlObject, "setValueInput", Q_ARG(QVariant, QString::number(ef.damage)));
        break;
    }
    case asn::EffectType::ChangeState: {
        const auto &ef = std::get<asn::ChangeState>(e);
        target = ef.target;
        targetSet = true;
        QMetaObject::invokeMethod(qmlObject, "setCardState", Q_ARG(QVariant, (int)ef.state));
        break;
    }
    case asn::EffectType::Backup: {
        const auto &ef = std::get<asn::Backup>(e);
        QMetaObject::invokeMethod(qmlObject, "setPower", Q_ARG(QVariant, QString::number(ef.power)));
        QMetaObject::invokeMethod(qmlObject, "setLevel", Q_ARG(QVariant, QString::number(ef.level)));
        break;
    }
    case asn::EffectType::FlipOver: {
        const auto &ef = std::get<asn::FlipOver>(e);
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)ef.number.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(ef.number.value)));
        break;
    }
    default:
        break;
    }
}

EffectImplComponent::~EffectImplComponent() {
    if (qmlObject)
        qmlObject->deleteLater();
}

void EffectImplComponent::init(QQuickItem *parent) {
    std::unordered_map<asn::EffectType, QString> components {
        { asn::EffectType::AttributeGain, "AttributeGain" },
        { asn::EffectType::ChooseCard, "ChooseCard" },
        { asn::EffectType::RevealCard, "RevealCard" },
        { asn::EffectType::Look, "Look" },
        { asn::EffectType::MoveCard, "MoveCard" },
        { asn::EffectType::SearchCard, "SearchCard" },
        { asn::EffectType::PayCost, "PayCost" },
        { asn::EffectType::AbilityGain, "AbilityGain" },
        { asn::EffectType::PerformEffect, "PerformEffect" },
        { asn::EffectType::NonMandatory, "NonMandatory" },
        { asn::EffectType::DealDamage, "DealDamage" },
        { asn::EffectType::ChangeState, "ChangeState" },
        { asn::EffectType::Backup, "Backup" },
        { asn::EffectType::FlipOver, "FlipOver" },
    };

    std::unordered_set<asn::EffectType> readyComponents {
        asn::EffectType::Shuffle,
        asn::EffectType::EarlyPlay,
        asn::EffectType::TriggerCheckTwice,
        asn::EffectType::StockSwap,
        asn::EffectType::DrawCard // temporary
    };

    if (readyComponents.contains(type))
        return;

    QQmlComponent component(qmlEngine(parent), "qrc:/qml/effects/" + components.at(type) + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setY(parent->property("effectImplY").toReal());

    initEffectByType(effect, type);

    switch (type) {
    case asn::EffectType::AttributeGain:
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, 1));
        QMetaObject::invokeMethod(qmlObject, "setValueInput", Q_ARG(QVariant, QString("0")));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(attrChanged(int)), this, SLOT(onAttrTypeChanged(int)));
        connect(qmlObject, SIGNAL(valueInputChanged(QString)), this, SLOT(onAttrChanged(QString)));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::ChooseCard:
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, static_cast<int>(asn::PlaceType::SpecificPlace)));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(placeTypeChanged(int)), this, SLOT(onPlaceTypeChanged(int)));
        connect(qmlObject, SIGNAL(executorChanged(int)), this, SLOT(onPlayerChanged(int)));
        break;
    case asn::EffectType::RevealCard:
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(revealTypeChanged(int)), this, SLOT(onRevealTypeChanged(int)));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::EffectType::Look:
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::EffectType::MoveCard:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editFrom()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(editTo()), this, SLOT(editTo()));
        connect(qmlObject, SIGNAL(orderChanged(int)), this, SLOT(onOrderChanged(int)));
        connect(qmlObject, SIGNAL(executorChanged(int)), this, SLOT(onPlayerChanged(int)));
        break;
    case asn::EffectType::SearchCard:
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::EffectType::PayCost:
        connect(qmlObject, SIGNAL(ifYouDo()), this, SLOT(editIfYouDo()));
        connect(qmlObject, SIGNAL(ifYouDont()), this, SLOT(editIfYouDont()));
        break;
    case asn::EffectType::NonMandatory:
        connect(qmlObject, SIGNAL(editEffects()), this, SLOT(editEffectsField()));
        connect(qmlObject, SIGNAL(ifYouDo()), this, SLOT(editIfYouDo()));
        connect(qmlObject, SIGNAL(ifYouDont()), this, SLOT(editIfYouDont()));
        break;
    case asn::EffectType::AbilityGain:
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, 1));
        QMetaObject::invokeMethod(qmlObject, "setNumber", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editAbilities()), this, SLOT(editAbilities()));
        connect(qmlObject, SIGNAL(numberChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::PerformEffect:
        QMetaObject::invokeMethod(qmlObject, "setEffectNum", Q_ARG(QVariant, QString("1")));
        QMetaObject::invokeMethod(qmlObject, "setEffectTimes", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(editAbilities()), this, SLOT(editAbilities()));
        connect(qmlObject, SIGNAL(effectNumChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        connect(qmlObject, SIGNAL(effectTimesChanged(QString)), this, SLOT(onNumOfTimesChanged(QString)));
        break;
    case asn::EffectType::DealDamage:
        QMetaObject::invokeMethod(qmlObject, "setValueInput", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(valueInputChanged(QString)), this, SLOT(onAttrChanged(QString)));
        break;
    case asn::EffectType::ChangeState:
        QMetaObject::invokeMethod(qmlObject, "setCardState", Q_ARG(QVariant, static_cast<int>(asn::State::Rested)));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(cardStateChanged(int)), this, SLOT(onCardStateChanged(int)));
        break;
    case asn::EffectType::Backup:
        QMetaObject::invokeMethod(qmlObject, "setPower", Q_ARG(QVariant, QString("1000")));
        QMetaObject::invokeMethod(qmlObject, "setLevel", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(powerChanged(QString)), this, SLOT(onAttrChanged(QString)));
        connect(qmlObject, SIGNAL(levelChanged(QString)), this, SLOT(onBackupLevelChanged(QString)));
        break;
    case asn::EffectType::FlipOver:
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString("4")));

        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(editEffects()), this, SLOT(editEffectsField()));
        break;
    default:
        break;
    }
}

void EffectImplComponent::editTarget() {
    if (targetSet)
        qmlTarget = std::make_unique<TargetComponent>(target, qmlObject);
    else
        qmlTarget = std::make_unique<TargetComponent>(qmlObject);

    connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &EffectImplComponent::targetReady);
    connect(qmlTarget.get(), &TargetComponent::close, this, &EffectImplComponent::destroyTarget);
}

void EffectImplComponent::destroyTarget() {
    qmlTarget.reset();
}

void EffectImplComponent::targetReady(const asn::Target &t) {
    targetSet = true;
    target = t;

    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto &e = std::get<asn::AttributeGain>(effect);
        e.target = target;
        break;
    }
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        e.targets[0].target = target;
        break;
    }
    case asn::EffectType::MoveCard: {
        auto &e = std::get<asn::MoveCard>(effect);
        e.target = target;
        break;
    }
    case asn::EffectType::AbilityGain: {
        auto &e = std::get<asn::AbilityGain>(effect);
        e.target = target;
        break;
    }
    case asn::EffectType::ChangeState: {
        auto &e = std::get<asn::ChangeState>(effect);
        e.target = target;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::onAttrTypeChanged(int value) {
    auto &e = std::get<asn::AttributeGain>(effect);
    e.type = static_cast<asn::AttributeType>(value);
    emit componentChanged(effect);
}

void EffectImplComponent::onAttrChanged(QString value) {
    bool ok;
    int val = value.toInt(&ok);
    if (!ok)
        return;
    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto &e = std::get<asn::AttributeGain>(effect);
        e.value = val;
        break;
    }
    case asn::EffectType::DealDamage: {
        auto &e = std::get<asn::DealDamage>(effect);
        e.damage = val;
        break;
    }
    case asn::EffectType::Backup: {
        auto &e = std::get<asn::Backup>(effect);
        e.power = val;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onBackupLevelChanged(QString value) {
    bool ok;
    int val = value.toInt(&ok);
    if (!ok)
        return;
    switch (type) {
    case asn::EffectType::Backup: {
        auto &e = std::get<asn::Backup>(effect);
        e.level = val;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onDurationChanged(int value) {
    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto &e = std::get<asn::AttributeGain>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::AbilityGain: {
        auto &e = std::get<asn::AbilityGain>(effect);
        e.duration = value;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::editPlace(std::optional<asn::Place> place) {
    auto &p = place.has_value() ? *place : getPlace(effect, type);
    qmlPlace = std::make_unique<PlaceComponent>(p, qmlObject);

    if (place.has_value())
        connect(qmlPlace.get(), &PlaceComponent::componentChanged, this, &EffectImplComponent::placeToReady);
    else
        connect(qmlPlace.get(), &PlaceComponent::componentChanged, this, &EffectImplComponent::placeReady);
    connect(qmlPlace.get(), &PlaceComponent::close, this, &EffectImplComponent::destroyPlace);
}

void EffectImplComponent::editTo() {
    auto &e = std::get<asn::MoveCard>(effect);
    editPlace(e.to[0]);
}

void EffectImplComponent::destroyPlace() {
    qmlPlace.reset();
}

void EffectImplComponent::placeReady(const asn::Place &p) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        e.targets[0].place = p;
        break;
    }
    case asn::EffectType::Look: {
        auto &e = std::get<asn::Look>(effect);
        e.place = p;
        break;
    }
    case asn::EffectType::MoveCard: {
        auto &e = std::get<asn::MoveCard>(effect);
        e.from = p;
        break;
    }
    case asn::EffectType::SearchCard: {
        auto &e = std::get<asn::SearchCard>(effect);
        e.place = p;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::placeToReady(const asn::Place &p) {
    auto &e = std::get<asn::MoveCard>(effect);
    e.to[0] = p;
    emit componentChanged(effect);
}

void EffectImplComponent::onPlaceTypeChanged(int value) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        e.targets[0].placeType = static_cast<asn::PlaceType>(value);
        if (e.targets[0].placeType == asn::PlaceType::Selection)
            e.targets[0].place = std::nullopt;
        else {
            auto defaultPlace = asn::Place();
            defaultPlace.owner = asn::Player::Player;
            defaultPlace.pos = asn::Position::NotSpecified;
            defaultPlace.zone = asn::Zone::Stage;
            e.targets[0].place = defaultPlace;
        }
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onPlayerChanged(int value) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        e.executor = static_cast<asn::Player>(value);
        break;
    }
    case asn::EffectType::MoveCard: {
        auto &e = std::get<asn::MoveCard>(effect);
        e.executor = static_cast<asn::Player>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::editCard() {
    qmlCard = std::make_unique<CardComponent>(getCard(effect, type), qmlObject);
    qmlCard->moveToTop();

    connect(qmlCard.get(), &CardComponent::componentChanged, this, &EffectImplComponent::cardReady);
    connect(qmlCard.get(), &CardComponent::close, this, &EffectImplComponent::destroyCard);
}

void EffectImplComponent::cardReady(const asn::Card &card_) {
    switch (type) {
    case asn::EffectType::RevealCard: {
        auto &e = std::get<asn::RevealCard>(effect);
        e.card = card_;
        break;
    }
    case asn::EffectType::SearchCard: {
        auto &e = std::get<asn::SearchCard>(effect);
        e.targets[0].cards[0] = card_;
        break;
    }
    case asn::EffectType::FlipOver: {
        auto &e = std::get<asn::FlipOver>(effect);
        e.forEach = card_;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::destroyCard() {
    qmlCard.reset();
}

void EffectImplComponent::onRevealTypeChanged(int value) {
    switch (type) {
    case asn::EffectType::RevealCard: {
        auto &e = std::get<asn::RevealCard>(effect);
        e.type = static_cast<asn::RevealType>(value);
        if (e.type == asn::RevealType::FromHand)
            e.card = asn::Card();
        else
            e.card = std::nullopt;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::onNumModifierChanged(int value) {
    switch (type) {
    case asn::EffectType::RevealCard: {
        auto &e = std::get<asn::RevealCard>(effect);
        e.number.mod = static_cast<asn::NumModifier>(value);
        break;
    }
    case asn::EffectType::Look: {
        auto &e = std::get<asn::Look>(effect);
        e.number.mod = static_cast<asn::NumModifier>(value);
        break;
    }
    case asn::EffectType::SearchCard: {
        auto &e = std::get<asn::SearchCard>(effect);
        e.targets[0].number.mod = static_cast<asn::NumModifier>(value);
        break;
    }
    case asn::EffectType::FlipOver: {
        auto &e = std::get<asn::FlipOver>(effect);
        e.number.mod = static_cast<asn::NumModifier>(value);
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::onNumValueChanged(QString value) {
    bool ok;
    int val = value.toInt(&ok);
    if (!ok)
        return;

    switch (type) {
    case asn::EffectType::RevealCard: {
        auto &e = std::get<asn::RevealCard>(effect);
        e.number.value = val;
        break;
    }
    case asn::EffectType::Look: {
        auto &e = std::get<asn::Look>(effect);
        e.number.value = val;
        break;
    }
    case asn::EffectType::SearchCard: {
        auto &e = std::get<asn::SearchCard>(effect);
        e.targets[0].number.value = val;
        break;
    }
    case asn::EffectType::AbilityGain: {
        auto &e = std::get<asn::AbilityGain>(effect);
        e.number = val;
        break;
    }
    case asn::EffectType::PerformEffect: {
        auto &e = std::get<asn::PerformEffect>(effect);
        e.numberOfEffects = val;
        break;
    }
    case asn::EffectType::FlipOver: {
        auto &e = std::get<asn::FlipOver>(effect);
        e.number.value = val;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::onOrderChanged(int value) {
    auto &e = std::get<asn::MoveCard>(effect);
    e.order = static_cast<asn::Order>(value);
    emit componentChanged(effect);
}

void EffectImplComponent::editEffectsField() {
    std::vector<asn::Effect> *eff = nullptr;
    switch (type) {
    case asn::EffectType::NonMandatory:
        eff = &std::get<asn::NonMandatory>(effect).effect;
        break;
    case asn::EffectType::FlipOver:
        eff = &std::get<asn::FlipOver>(effect).effect;
        break;
    default: assert(false);
    }

    editEffects(*eff);
    connect(qmlEffects.get(), &ArrayOfEffectsComponent::componentChanged, this, &EffectImplComponent::effectsReady);
}

void EffectImplComponent::editIfYouDo() {
    std::vector<asn::Effect> *eff = nullptr;
    switch (type) {
    case asn::EffectType::PayCost:
        eff = &std::get<asn::PayCost>(effect).ifYouDo;
        break;
    case asn::EffectType::NonMandatory:
        eff = &std::get<asn::NonMandatory>(effect).ifYouDo;
        break;
    default: assert(false);
    }

    editEffects(*eff);
    connect(qmlEffects.get(), &ArrayOfEffectsComponent::componentChanged, this, &EffectImplComponent::ifYouDoReady);
}

void EffectImplComponent::editIfYouDont() {
    std::vector<asn::Effect> *eff = nullptr;
    switch (type) {
    case asn::EffectType::PayCost:
        eff = &std::get<asn::PayCost>(effect).ifYouDont;
        break;
    case asn::EffectType::NonMandatory:
        eff = &std::get<asn::NonMandatory>(effect).ifYouDont;
        break;
    default: assert(false);
    }

    editEffects(*eff);
    connect(qmlEffects.get(), &ArrayOfEffectsComponent::componentChanged, this, &EffectImplComponent::ifYouDontReady);
}

void EffectImplComponent::editEffects(const std::vector<asn::Effect> &effects) {
    qmlEffects = std::make_unique<ArrayOfEffectsComponent>(effects, qmlObject);

    connect(qmlEffects.get(), &ArrayOfEffectsComponent::close, this, &EffectImplComponent::destroyEffects);
}

void EffectImplComponent::effectsReady(const std::vector<asn::Effect> &effects) {
    switch (type) {
    case asn::EffectType::NonMandatory: {
        auto &e = std::get<asn::NonMandatory>(effect);
        e.effect = effects;
        break;
    }
    case asn::EffectType::FlipOver: {
        auto &e = std::get<asn::FlipOver>(effect);
        e.effect = effects;
        break;
    }
    default: assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::ifYouDoReady(const std::vector<asn::Effect> &effects) {
    switch (type) {
    case asn::EffectType::PayCost: {
        auto &e = std::get<asn::PayCost>(effect);
        e.ifYouDo = effects;
        break;
    }
    case asn::EffectType::NonMandatory: {
        auto &e = std::get<asn::NonMandatory>(effect);
        e.ifYouDo = effects;
        break;
    }
    default: assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::ifYouDontReady(const std::vector<asn::Effect> &effects) {
    switch (type) {
    case asn::EffectType::PayCost: {
        auto &e = std::get<asn::PayCost>(effect);
        e.ifYouDont = effects;
        break;
    }
    case asn::EffectType::NonMandatory: {
        auto &e = std::get<asn::NonMandatory>(effect);
        e.ifYouDont = effects;
        break;
    }
    default: assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::destroyEffects() {
    qmlEffects.reset();
}

void EffectImplComponent::editAbilities() {
    std::vector<asn::Ability> abilities;
    switch (type) {
    case asn::EffectType::AbilityGain:
        abilities = std::get<asn::AbilityGain>(effect).abilities;
        break;
    case asn::EffectType::PerformEffect: {
        auto e = std::get<asn::PerformEffect>(effect);
        for (int i = 0; i < e.effects.size(); ++i) {
            abilities.push_back(asn::Ability());
            abilities.back().type = asn::AbilityType::Event;
            abilities.back().ability = e.effects[i];
        }
        break;
    }
    default: break;
    }

    qmlAbilities = std::make_unique<ArrayOfAbilitiesComponent>(abilities, qmlObject);
    if (type == asn::EffectType::PerformEffect)
        qmlAbilities->fixEventAbility();

    connect(qmlAbilities.get(), &ArrayOfAbilitiesComponent::close, this, &EffectImplComponent::destroyAbilities);
    connect(qmlAbilities.get(), &ArrayOfAbilitiesComponent::componentChanged, this, &EffectImplComponent::abilitiesReady);
}

void EffectImplComponent::destroyAbilities() {
    qmlAbilities.reset();
}

void EffectImplComponent::abilitiesReady(const std::vector<asn::Ability> &a) {
    if (type == asn::EffectType::PerformEffect) {
        std::vector<asn::EventAbility> ea;
        for (size_t i = 0; i < a.size(); ++i) {
            assert(a[i].type == asn::AbilityType::Event);
            ea.push_back(std::get<asn::EventAbility>(a[i].ability));
        }
        auto &e = std::get<asn::PerformEffect>(effect);
        e.effects = ea;
        emit componentChanged(effect);
        return;
    }
    auto &e = std::get<asn::AbilityGain>(effect);
    e.abilities = a;
    emit componentChanged(effect);
}

void EffectImplComponent::onNumOfTimesChanged(QString value) {
    bool ok;
    int val = value.toInt(&ok);
    if (!ok)
        return;

    auto &e = std::get<asn::PerformEffect>(effect);
    e.numberOfTimes = val;
    emit componentChanged(effect);
}

void EffectImplComponent::onCardStateChanged(int value) {
    switch (type) {
    case asn::EffectType::ChangeState: {
        auto &e = std::get<asn::ChangeState>(effect);
        e.state = static_cast<asn::State>(value);
        break;
    }
    default: assert(false);
    }
    emit componentChanged(effect);
}
