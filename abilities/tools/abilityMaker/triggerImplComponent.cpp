#include "triggerImplComponent.h"

#include <QQmlContext>

void initTriggerByType(TriggerImplComponent::VarTrigger &trigger, asn::TriggerType type) {
    switch (type) {
    case asn::TriggerType::OnZoneChange: {
        auto tr = asn::ZoneChangeTrigger();

        tr.target.push_back(asn::Target());
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnPlay:
        trigger = asn::OnPlayTrigger();
        break;
    case asn::TriggerType::OnAttack:
        trigger = asn::OnAttackTrigger();
        break;
    case asn::TriggerType::OnStateChange: {
        auto tr = asn::StateChangeTrigger();
        tr.state = asn::State::Reversed;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnTriggerReveal:
        trigger = asn::TriggerRevealTrigger();
        break;
    case asn::TriggerType::OnPhaseEvent: {
        auto tr = asn::PhaseTrigger();
        tr.phase = asn::Phase::Mulligan;
        tr.state = asn::PhaseState::Start;
        tr.player = asn::Player::Player;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnBackupOfThis:
    case asn::TriggerType::OnEndOfThisCardsAttack:
    case asn::TriggerType::OnEndOfThisTurn:
    case asn::TriggerType::OnOppCharPlacedByStandbyTriggerReveal:
    case asn::TriggerType::OtherTrigger:
        trigger = std::monostate();
        break;
    default:
        assert(false);
    }
}

TriggerImplComponent::TriggerImplComponent(asn::TriggerType type, QQuickItem *parent)
    : type(type) {
    init(parent);
}

TriggerImplComponent::TriggerImplComponent(asn::TriggerType type, const VarTrigger &tr, QQuickItem *parent)
    : type (type) {
    init(parent);

    trigger = tr;
    switch (type) {
    case asn::TriggerType::OnZoneChange: {
        const auto &trImpl = std::get<asn::ZoneChangeTrigger>(tr);
        QMetaObject::invokeMethod(qmlObject, "setFrom", Q_ARG(QVariant, static_cast<int>(trImpl.from)));
        QMetaObject::invokeMethod(qmlObject, "setTo", Q_ARG(QVariant, static_cast<int>(trImpl.to)));
        target = trImpl.target[0];
        targetSet = true;
        break;
    }
    case asn::TriggerType::OnPlay: {
        const auto &trImpl = std::get<asn::OnPlayTrigger>(tr);
        target = trImpl.target;
        targetSet = true;
        break;
    }
    case asn::TriggerType::OnAttack: {
        const auto &trImpl = std::get<asn::OnAttackTrigger>(tr);
        target = trImpl.target;
        targetSet = true;
        break;
    }
    case asn::TriggerType::OnPhaseEvent: {
        const auto &trImpl = std::get<asn::PhaseTrigger>(tr);
        QMetaObject::invokeMethod(qmlObject, "setPhaseState", Q_ARG(QVariant, static_cast<int>(trImpl.state)));
        QMetaObject::invokeMethod(qmlObject, "setPhase", Q_ARG(QVariant, static_cast<int>(trImpl.phase)));
        QMetaObject::invokeMethod(qmlObject, "setPlayer", Q_ARG(QVariant, static_cast<int>(trImpl.player)));
        break;
    }
    case asn::TriggerType::OnTriggerReveal: {
        const auto &trImpl = std::get<asn::TriggerRevealTrigger>(tr);
        card = trImpl.card;
        cardSet = true;
        break;
    }
    case asn::TriggerType::OnStateChange: {
        const auto &trImpl = std::get<asn::StateChangeTrigger>(tr);
        QMetaObject::invokeMethod(qmlObject, "setCardState", Q_ARG(QVariant, static_cast<int>(trImpl.state)));
        target = trImpl.target;
        targetSet = true;
        break;
    }
    default:
        break;
    }
}

void TriggerImplComponent::init(QQuickItem *parent) {
    std::unordered_map<asn::TriggerType, QString> components {
        { asn::TriggerType::OnZoneChange, "ZoneChangeTrigger" },
        { asn::TriggerType::OnPlay, "OnPlayTrigger" },
        { asn::TriggerType::OnAttack, "OnAttackTrigger" },
        { asn::TriggerType::OnPhaseEvent, "PhaseTrigger" },
        { asn::TriggerType::OnTriggerReveal, "TriggerReveal" },
        { asn::TriggerType::OnStateChange, "StateChangeTrigger" }
    };
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/triggers/" + components.at(type) + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setY(parent->property("triggerImplY").toReal());

    initTriggerByType(trigger, type);

    switch (type) {
    case asn::TriggerType::OnZoneChange:
        connect(qmlObject, SIGNAL(fromChanged(int)), this, SLOT(setFrom(int)));
        connect(qmlObject, SIGNAL(toChanged(int)), this, SLOT(setTo(int)));
    case asn::TriggerType::OnPlay:
    case asn::TriggerType::OnAttack:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        break;
    case asn::TriggerType::OnPhaseEvent:
        connect(qmlObject, SIGNAL(phaseStateChanged(int)), this, SLOT(setPhaseState(int)));
        connect(qmlObject, SIGNAL(phaseChanged(int)), this, SLOT(setPhase(int)));
        connect(qmlObject, SIGNAL(ownerChanged(int)), this, SLOT(setOwner(int)));
        break;
    case asn::TriggerType::OnStateChange:
        connect(qmlObject, SIGNAL(cardStateChanged(int)), this, SLOT(setState(int)));
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        QMetaObject::invokeMethod(qmlObject, "setCardState", Q_ARG(QVariant, static_cast<int>(asn::State::Reversed)));
        break;
    case asn::TriggerType::OnTriggerReveal:
        connect(qmlObject, SIGNAL(editCard()), this, SLOT(editCard()));
        connect(qmlObject, SIGNAL(clearCard()), this, SLOT(clearCard()));
        break;
    default:
        break;
    }
}

TriggerImplComponent::~TriggerImplComponent() {
    qmlObject->deleteLater();
}

void TriggerImplComponent::componentReady() {
    emit componentChanged(trigger);
}

void TriggerImplComponent::setFrom(int index) {
    auto &trig = std::get<asn::ZoneChangeTrigger>(trigger);
    trig.from = static_cast<asn::Zone>(index);
    emit componentChanged(trigger);
}

void TriggerImplComponent::setTo(int index) {
    auto &trig = std::get<asn::ZoneChangeTrigger>(trigger);
    trig.to = static_cast<asn::Zone>(index);
    emit componentChanged(trigger);
}

void TriggerImplComponent::editTarget() {
    if (targetSet)
        qmlTarget = std::make_unique<TargetComponent>(target, qmlObject);
    else
        qmlTarget = std::make_unique<TargetComponent>(qmlObject);

    connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &TriggerImplComponent::targetReady);
    connect(qmlTarget.get(), &TargetComponent::close, this, &TriggerImplComponent::destroyTarget);
}

void TriggerImplComponent::destroyTarget() {
    qmlTarget.reset();
}

void TriggerImplComponent::targetReady(const asn::Target &t) {
    targetSet = true;
    target = t;
    switch (type) {
    case asn::TriggerType::OnZoneChange: {
        auto &trig = std::get<asn::ZoneChangeTrigger>(trigger);
        trig.target.clear();
        trig.target.push_back(t);
        break;
    }
    case asn::TriggerType::OnPlay: {
        auto &trig = std::get<asn::OnPlayTrigger>(trigger);
        trig.target = t;
        break;
    }
    case asn::TriggerType::OnAttack: {
        auto &trig = std::get<asn::OnAttackTrigger>(trigger);
        trig.target = t;
        break;
    }
    case asn::TriggerType::OnStateChange: {
        auto &trig = std::get<asn::StateChangeTrigger>(trigger);
        trig.target = t;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(trigger);
}

void TriggerImplComponent::setPhaseState(int index) {
    phaseTrigger.state = static_cast<asn::PhaseState>(index);
    trigger = phaseTrigger;
    emit componentChanged(trigger);
}

void TriggerImplComponent::setPhase(int index) {
    phaseTrigger.phase = static_cast<asn::Phase>(index);
    trigger = phaseTrigger;
    emit componentChanged(trigger);
}

void TriggerImplComponent::setOwner(int index) {
    phaseTrigger.player = static_cast<asn::Player>(index);
    trigger = phaseTrigger;
    emit componentChanged(trigger);
}

void TriggerImplComponent::setState(int index) {
    auto &tr = std::get<asn::StateChangeTrigger>(trigger);
    tr.state = static_cast<asn::State>(index);
    emit componentChanged(trigger);
}

void TriggerImplComponent::editCard() {
    if (cardSet)
        qmlCard = std::make_unique<CardComponent>(card, qmlObject);
    else
        qmlCard = std::make_unique<CardComponent>(qmlObject);
    qmlCard->moveToTop();

    connect(qmlCard.get(), &CardComponent::componentChanged, this, &TriggerImplComponent::cardReady);
    connect(qmlCard.get(), &CardComponent::close, this, &TriggerImplComponent::destroyCard);
}

void TriggerImplComponent::clearCard() {
    card = asn::Card();
}

void TriggerImplComponent::cardReady(const asn::Card &card_) {
    card = card_;
    cardSet = true;
    switch (type) {
    case asn::TriggerType::OnTriggerReveal: {
        auto &tr = std::get<asn::TriggerRevealTrigger>(trigger);
        tr.card = card;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(trigger);
}

void TriggerImplComponent::destroyCard() {
    qmlCard.reset();
}
