#include "effectImplComponent.h"

#include <unordered_map>
#include <unordered_set>

#include <QQmlContext>
#include <QString>

#include "arrayOfEffectsComponent.h"
#include "arrayOfAbilitiesComponent.h"
#include "triggerImplComponent.h"

namespace {
const asn::Place defaultPlace{asn::Position::NotSpecified, asn::Zone::Stage, asn::Player::Player};
const asn::Target defaultTarget{asn::TargetType::ThisCard};
asn::TargetAndPlace defaultTargetAndPlace() {
    auto tp = asn::TargetAndPlace();
    tp.placeType = asn::PlaceType::SpecificPlace;
    tp.place = defaultPlace;
    tp.target = defaultTarget;
    return tp;
}
}

void initEffectByType(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    auto defaultNum = asn::Number();
    defaultNum.mod = asn::NumModifier::ExactMatch;
    defaultNum.value = 1;

    auto defaultChooseCard = asn::ChooseCard();
    auto defaultTargetAndPlace = asn::TargetAndPlace();
    defaultTargetAndPlace.placeType = asn::PlaceType::SpecificPlace;
    defaultTargetAndPlace.place = defaultPlace;
    defaultTargetAndPlace.target = defaultTarget;
    defaultChooseCard.executor = asn::Player::Player;
    defaultChooseCard.targets.push_back(defaultTargetAndPlace);

    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto e = asn::AttributeGain();
        e.type = asn::AttributeType::Power;
        e.gainType = asn::ValueType::Raw;
        e.value = 0;
        e.duration = 1;
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::ChooseCard: {
        effect = defaultChooseCard;
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
        auto place = defaultPlace;
        place.pos = asn::Position::Top;
        place.zone = asn::Zone::Deck;
        e.number = defaultNum;
        e.place = place;
        e.valueType = asn::ValueType::Raw;
        effect = e;
        break;
    }
    case asn::EffectType::MoveCard: {
        auto e = asn::MoveCard();
        e.executor = asn::Player::Player;
        e.from = defaultPlace;
        e.to.push_back(defaultPlace);
        e.order = asn::Order::NotSpecified;
        e.target = defaultTarget;
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
        e.target = defaultTarget;
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
    case asn::EffectType::ShotTriggerDamage:
    case asn::EffectType::DealDamage: {
        auto e = asn::DealDamage();
        e.damageType = asn::ValueType::Raw;
        e.damage = 1;
        effect = e;
        break;
    }
    case asn::EffectType::ChangeState: {
        auto e = asn::ChangeState();
        e.target = defaultTarget;
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
    case asn::EffectType::CannotAttack: {
        auto e = asn::CannotAttack();
        e.type = asn::AttackType::Any;
        e.duration = 0;
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::SideAttackWithoutPenalty: {
        auto e = asn::SideAttackWithoutPenalty();
        e.duration = 0;
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::AddMarker: {
        auto e = asn::AddMarker();
        e.target = defaultTarget;
        e.from = defaultPlace;
        e.destination = defaultTarget;
        e.orientation = asn::FaceOrientation::FaceDown;
        e.withMarkers = false;
        effect = e;
        break;
    }
    case asn::EffectType::RemoveMarker: {
        auto e = asn::RemoveMarker();
        e.targetMarker = defaultTarget;
        e.markerBearer = defaultTarget;
        e.place = defaultPlace;
        effect = e;
        break;
    }
    case asn::EffectType::MoveWrToDeck: {
        auto e = asn::MoveWrToDeck();
        e.executor = asn::Player::Player;
        effect = e;
        break;
    }
    case asn::EffectType::CannotUseBackupOrEvent: {
        auto e = asn::CannotUseBackupOrEvent();
        e.what = asn::BackupOrEvent::Backup;
        e.player = asn::Player::Player;
        e.duration = 0;
        effect = e;
        break;
    }
    case asn::EffectType::SwapCards: {
        auto e = asn::SwapCards();
        e.first = defaultChooseCard;
        e.second = defaultChooseCard;
        effect = e;
        break;
    }
    case asn::EffectType::OpponentAutoCannotDealDamage: {
        auto e = asn::OpponentAutoCannotDealDamage();
        e.duration = 0;
        effect = e;
        break;
    }
    case asn::EffectType::CannotBecomeReversed: {
        auto e = asn::CannotBecomeReversed();
        e.duration = 0;
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::CannotMove: {
        auto e = asn::CannotMove();
        e.duration = 0;
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::CannotStand: {
        auto e = asn::CannotStand();
        e.duration = 0;
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::CannotBeChosen: {
        auto e = asn::CannotBeChosen();
        e.duration = 0;
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::PutOnStageRested: {
        auto e = asn::PutOnStageRested();
        e.target = defaultTarget;
        e.from = defaultPlace;
        e.to = asn::Position::NotSpecified;
        effect = e;
        break;
    }
    case asn::EffectType::TriggerIconGain: {
        auto e = asn::TriggerIconGain();
        e.target = defaultTarget;
        e.duration = 0;
        e.triggerIcon = asn::TriggerIcon::Soul;
        effect = e;
        break;
    }
    case asn::EffectType::CanPlayWithoutColorRequirement: {
        auto e = asn::CanPlayWithoutColorRequirement();
        e.target = defaultTarget;
        e.duration = 0;
        effect = e;
        break;
    }
    case asn::EffectType::DelayedAbility: {
        auto e = asn::DelayedAbility();
        e.ability = std::make_shared<asn::AutoAbility>();
        e.duration = 1;
        effect = e;
        break;
    }
    case asn::EffectType::CostSubstitution: {
        auto e = asn::CostSubstitution();
        effect = e;
        break;
    }
    case asn::EffectType::StockSwap: {
        auto e = asn::StockSwap();
        e.zone = asn::Zone::WaitingRoom;
        effect = e;
        break;
    }
    case asn::EffectType::SkipPhase: {
        auto e = asn::SkipPhase();
        e.skipUntil = asn::Phase::EndPhase;
        effect = e;
        break;
    }
    case asn::EffectType::ChooseTrait: {
        auto e = asn::ChooseTrait();
        e.target = defaultTarget;
        effect = e;
        break;
    }
    case asn::EffectType::TraitModification: {
        auto e = asn::TraitModification();
        e.type = asn::TraitModificationType::TraitGain;
        e.target = defaultTargetAndPlace;
        e.traitType = asn::TraitType::Value;
        e.duration = 1;
        effect = e;
        break;
    }
    case asn::EffectType::OtherEffect: {
        effect = asn::OtherEffect();
        break;
    }
    case asn::EffectType::TriggerCheckTwice:
    case asn::EffectType::EarlyPlay:
    case asn::EffectType::Standby:
    case asn::EffectType::CannotPlay:
    case asn::EffectType::CharAutoCannotDealDamage:
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
    case asn::EffectType::AddMarker: {
        const auto &e = std::get<asn::AddMarker>(effect);
        return e.from;
    }
    case asn::EffectType::RemoveMarker: {
        const auto &e = std::get<asn::RemoveMarker>(effect);
        return e.place;
    }
    case asn::EffectType::PutOnStageRested: {
        const auto &e = std::get<asn::PutOnStageRested>(effect);
        return e.from;
    }
    case asn::EffectType::TraitModification: {
        const auto &e = std::get<asn::TraitModification>(effect);
        return *e.target.place;
    }
    default:
        assert(false);
    }
    static auto p = asn::Place();
    return p;
}
const std::optional<asn::Place> getPlace2(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        const auto &e = std::get<asn::ChooseCard>(effect);
        if (e.targets.size() <= 1)
            return std::nullopt;
        return e.targets[1].place;
    }
    }
    return std::nullopt;
}
const asn::Target& getTarget(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    switch (type) {
    case asn::EffectType::AttributeGain: {
        const auto &e = std::get<asn::AttributeGain>(effect);
        return e.target;
    }
    case asn::EffectType::ChooseCard: {
        const auto &e = std::get<asn::ChooseCard>(effect);
        return e.targets[0].target;
    }
    case asn::EffectType::AbilityGain: {
        const auto &e = std::get<asn::AbilityGain>(effect);
        return e.target;
    }
    case asn::EffectType::MoveCard: {
        const auto &e = std::get<asn::MoveCard>(effect);
        return e.target;
    }
    case asn::EffectType::ChangeState: {
        const auto &e = std::get<asn::ChangeState>(effect);
        return e.target;
    }
    case asn::EffectType::CannotAttack: {
        const auto &e = std::get<asn::CannotAttack>(effect);
        return e.target;
    }
    case asn::EffectType::SideAttackWithoutPenalty: {
        const auto &e = std::get<asn::SideAttackWithoutPenalty>(effect);
        return e.target;
    }
    case asn::EffectType::AddMarker: {
        const auto &e = std::get<asn::AddMarker>(effect);
        return e.target;
    }
    case asn::EffectType::RemoveMarker: {
        const auto &e = std::get<asn::RemoveMarker>(effect);
        return e.targetMarker;
    }
    case asn::EffectType::CannotBecomeReversed: {
        const auto &e = std::get<asn::CannotBecomeReversed>(effect);
        return e.target;
    }
    case asn::EffectType::CannotMove: {
        const auto &e = std::get<asn::CannotMove>(effect);
        return e.target;
    }
    case asn::EffectType::PutOnStageRested: {
        const auto &e = std::get<asn::PutOnStageRested>(effect);
        return e.target;
    }
    case asn::EffectType::CannotStand: {
        const auto &e = std::get<asn::CannotStand>(effect);
        return e.target;
    }
    case asn::EffectType::CannotBeChosen: {
        const auto &e = std::get<asn::CannotBeChosen>(effect);
        return e.target;
    }
    case asn::EffectType::TriggerIconGain: {
        const auto &e = std::get<asn::TriggerIconGain>(effect);
        return e.target;
    }
    case asn::EffectType::CanPlayWithoutColorRequirement: {
        const auto &e = std::get<asn::CanPlayWithoutColorRequirement>(effect);
        return e.target;
    }
    case asn::EffectType::ChooseTrait: {
        const auto &e = std::get<asn::ChooseTrait>(effect);
        return e.target;
    }
    case asn::EffectType::TraitModification: {
        const auto &e = std::get<asn::TraitModification>(effect);
        return e.target.target;
    }
    default:
        assert(false);
    }
    static auto t = asn::Target();
    return t;
}
const std::optional<asn::Target> getTarget2(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        const auto &e = std::get<asn::ChooseCard>(effect);
        if (e.targets.size() <= 1)
            return std::nullopt;
        return e.targets[1].target;
    }
    }
    return std::nullopt;
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
const std::optional<asn::Multiplier> getMultiplier(EffectImplComponent::VarEffect &effect, asn::EffectType type) {
    switch (type) {
    case asn::EffectType::AttributeGain: {
        const auto &e = std::get<asn::AttributeGain>(effect);
        return e.modifier;
    }
    case asn::EffectType::Look: {
        const auto &e = std::get<asn::Look>(effect);
        return e.multiplier;
    }
    case asn::EffectType::ShotTriggerDamage:
    case asn::EffectType::DealDamage: {
        const auto &e = std::get<asn::DealDamage>(effect);
        return e.modifier;
    }
    default:
        assert(false);
    }
    throw std::runtime_error("unhandled EffectType in multiplier");
}
asn::Multiplier getDefaultMultiplier() {
    asn::Multiplier m;
    m.type = asn::MultiplierType::ForEach;
    auto mm = asn::ForEachMultiplier();
    mm.target = std::make_shared<asn::Target>();
    mm.placeType = asn::PlaceType::Selection;
    m.specifier = mm;
    return m;
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

        QMetaObject::invokeMethod(qmlObject, "setAttrType", Q_ARG(QVariant, (int)ef.type));
        QMetaObject::invokeMethod(qmlObject, "setValueInput", Q_ARG(QVariant, QString::number(ef.value)));
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, ef.duration));
        QMetaObject::invokeMethod(qmlObject, "setGainType", Q_ARG(QVariant, (int)(ef.gainType)));
        break;
    }
    case asn::EffectType::ChooseCard: {
        const auto &ef = std::get<asn::ChooseCard>(e);

        if (ef.targets.size() > 1) {
            QMetaObject::invokeMethod(qmlObject, "setSecondTarget", Q_ARG(QVariant, (int)ef.targets[1].placeType));
        }
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, (int)ef.targets[0].placeType));
        QMetaObject::invokeMethod(qmlObject, "setExecutor", Q_ARG(QVariant, (int)ef.executor));
        break;
    }
    case asn::EffectType::MoveCard: {
        const auto &ef = std::get<asn::MoveCard>(e);

        QMetaObject::invokeMethod(qmlObject, "setOrder", Q_ARG(QVariant, (int)ef.order));
        QMetaObject::invokeMethod(qmlObject, "setExecutor", Q_ARG(QVariant, (int)ef.executor));
        break;
    }
    case asn::EffectType::RevealCard: {
        const auto &ef = std::get<asn::RevealCard>(e);

        QMetaObject::invokeMethod(qmlObject, "setRevealType", Q_ARG(QVariant, (int)ef.type));
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)ef.number.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(ef.number.value)));
        break;
    }
    case asn::EffectType::Look: {
        const auto &ef = std::get<asn::Look>(e);
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)ef.number.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(ef.number.value)));
        QMetaObject::invokeMethod(qmlObject, "setValueType", Q_ARG(QVariant, (int)(ef.valueType)));
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

        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        QMetaObject::invokeMethod(qmlObject, "setNumber", Q_ARG(QVariant, QString::number(ef.number)));
        break;
    }
    case asn::EffectType::PerformEffect: {
        const auto &ef = std::get<asn::PerformEffect>(e);
        QMetaObject::invokeMethod(qmlObject, "setEffectNum", Q_ARG(QVariant, QString::number(ef.numberOfEffects)));
        QMetaObject::invokeMethod(qmlObject, "setEffectTimes", Q_ARG(QVariant, QString::number(ef.numberOfTimes)));
        if (ef.numberOfTimes == 0)
            QMetaObject::invokeMethod(qmlObject, "setRepeat");
        break;
    }
    case asn::EffectType::ShotTriggerDamage:
    case asn::EffectType::DealDamage: {
        const auto &ef = std::get<asn::DealDamage>(e);
        QMetaObject::invokeMethod(qmlObject, "setValueInput", Q_ARG(QVariant, QString::number(ef.damage)));
        QMetaObject::invokeMethod(qmlObject, "setDamageType", Q_ARG(QVariant, (int)(ef.damageType)));
        break;
    }
    case asn::EffectType::ChangeState: {
        const auto &ef = std::get<asn::ChangeState>(e);

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
    case asn::EffectType::DrawCard: {
        const auto &ef = std::get<asn::DrawCard>(e);
        QMetaObject::invokeMethod(qmlObject, "setExecutor", Q_ARG(QVariant, (int)ef.executor));
        QMetaObject::invokeMethod(qmlObject, "setNumModifier", Q_ARG(QVariant, (int)ef.value.mod));
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString::number(ef.value.value)));
        break;
    }
    case asn::EffectType::CannotAttack: {
        const auto &ef = std::get<asn::CannotAttack>(e);
        QMetaObject::invokeMethod(qmlObject, "setAttackType", Q_ARG(QVariant, (int)ef.type));
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::SideAttackWithoutPenalty: {
        const auto &ef = std::get<asn::SideAttackWithoutPenalty>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::AddMarker: {
        const auto &ef = std::get<asn::AddMarker>(e);
        QMetaObject::invokeMethod(qmlObject, "setFaceOrientation", Q_ARG(QVariant, (int)ef.orientation));
        QMetaObject::invokeMethod(qmlObject, "setWithMarkers", Q_ARG(QVariant, (int)ef.withMarkers));
        break;
    }
    case asn::EffectType::Shuffle: {
        const auto &ef = std::get<asn::Shuffle>(e);
        QMetaObject::invokeMethod(qmlObject, "setPlayer", Q_ARG(QVariant, (int)ef.owner));
        QMetaObject::invokeMethod(qmlObject, "setZone", Q_ARG(QVariant, (int)ef.zone));
        break;
    }
    case asn::EffectType::MoveWrToDeck: {
        const auto &ef = std::get<asn::MoveWrToDeck>(e);
        QMetaObject::invokeMethod(qmlObject, "setExecutor", Q_ARG(QVariant, (int)ef.executor));
        break;
    }
    case asn::EffectType::CannotUseBackupOrEvent: {
        const auto &ef = std::get<asn::CannotUseBackupOrEvent>(e);
        QMetaObject::invokeMethod(qmlObject, "setBackupOrEvent", Q_ARG(QVariant, (int)ef.what));
        QMetaObject::invokeMethod(qmlObject, "setPlayer", Q_ARG(QVariant, (int)ef.player));
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::OpponentAutoCannotDealDamage: {
        const auto &ef = std::get<asn::OpponentAutoCannotDealDamage>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
    }
    case asn::EffectType::CannotBecomeReversed: {
        const auto &ef = std::get<asn::CannotBecomeReversed>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::CannotMove: {
        const auto &ef = std::get<asn::CannotMove>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::PutOnStageRested: {
        const auto &ef = std::get<asn::PutOnStageRested>(e);
        QMetaObject::invokeMethod(qmlObject, "setPosition", Q_ARG(QVariant, (int)ef.to));
        break;
    }
    case asn::EffectType::CannotStand: {
        const auto &ef = std::get<asn::CannotStand>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::CannotBeChosen: {
        const auto &ef = std::get<asn::CannotBeChosen>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::TriggerIconGain: {
        const auto &ef = std::get<asn::TriggerIconGain>(e);
        QMetaObject::invokeMethod(qmlObject, "setTriggerIcon", Q_ARG(QVariant, (int)ef.triggerIcon));
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::CanPlayWithoutColorRequirement: {
        const auto &ef = std::get<asn::CanPlayWithoutColorRequirement>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::DelayedAbility: {
        const auto &ef = std::get<asn::DelayedAbility>(e);
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        break;
    }
    case asn::EffectType::StockSwap: {
        const auto &ef = std::get<asn::StockSwap>(e);
        QMetaObject::invokeMethod(qmlObject, "setZone", Q_ARG(QVariant, (int)ef.zone));
        break;
    }
    case asn::EffectType::SkipPhase: {
        const auto &ef = std::get<asn::SkipPhase>(e);
        QMetaObject::invokeMethod(qmlObject, "setPhase", Q_ARG(QVariant, (int)ef.skipUntil));
        break;
    }
    case asn::EffectType::TraitModification: {
        const auto &ef = std::get<asn::TraitModification>(e);
        QMetaObject::invokeMethod(qmlObject, "setType", Q_ARG(QVariant, (int)ef.type));
        QMetaObject::invokeMethod(qmlObject, "setTraitType", Q_ARG(QVariant, (int)ef.traitType));
        QMetaObject::invokeMethod(qmlObject, "setDuration", Q_ARG(QVariant, (int)ef.duration));
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, (int)ef.target.placeType));
        break;
    }
    case asn::EffectType::OtherEffect: {
        const auto &ef = std::get<asn::OtherEffect>(e);
        QMetaObject::invokeMethod(qmlObject, "setCardCode", Q_ARG(QVariant, QString::fromStdString(ef.cardCode)));
        QMetaObject::invokeMethod(qmlObject, "setEffectId", Q_ARG(QVariant, QString::number(ef.effectId)));
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
        { asn::EffectType::DrawCard, "DrawCard" },
        { asn::EffectType::CannotAttack, "CannotAttack" },
        { asn::EffectType::SideAttackWithoutPenalty, "TargetDurationEffect" },
        { asn::EffectType::AddMarker, "AddMarker" },
        { asn::EffectType::RemoveMarker, "RemoveMarker" },
        { asn::EffectType::Shuffle, "Shuffle" },
        { asn::EffectType::MoveWrToDeck, "MoveWrToDeck" },
        { asn::EffectType::CannotUseBackupOrEvent, "CannotUseBackup" },
        { asn::EffectType::SwapCards, "SwapCards" },
        { asn::EffectType::OpponentAutoCannotDealDamage, "TargetDurationEffect" },
        { asn::EffectType::CannotBecomeReversed, "TargetDurationEffect" },
        { asn::EffectType::CannotMove, "TargetDurationEffect" },
        { asn::EffectType::PutOnStageRested, "PutOnStageRested" },
        { asn::EffectType::CannotStand, "TargetDurationEffect" },
        { asn::EffectType::CannotBeChosen, "TargetDurationEffect" },
        { asn::EffectType::TriggerIconGain, "TriggerIconGain" },
        { asn::EffectType::CanPlayWithoutColorRequirement, "TargetDurationEffect" },
        { asn::EffectType::ShotTriggerDamage, "DealDamage" },
        { asn::EffectType::DelayedAbility, "DelayedAbility" },
        { asn::EffectType::CostSubstitution, "CostSubstitution" },
        { asn::EffectType::StockSwap, "StockSwap" },
        { asn::EffectType::SkipPhase, "SkipPhase" },
        { asn::EffectType::ChooseTrait, "TargetDurationEffect" },
        { asn::EffectType::TraitModification, "TraitModification" },
        { asn::EffectType::OtherEffect, "OtherEffect" }
    };

    std::unordered_set<asn::EffectType> readyComponents {
        asn::EffectType::EarlyPlay,
        asn::EffectType::TriggerCheckTwice,
        asn::EffectType::Standby,
        asn::EffectType::CannotPlay,
        asn::EffectType::CharAutoCannotDealDamage
    };

    if (readyComponents.contains(type))
        return;

    if (!components.contains(type))
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
        connect(qmlObject, SIGNAL(gainTypeChanged(int)), this, SLOT(onValueTypeChanged(int)));
        connect(qmlObject, SIGNAL(editMultiplier()), this, SLOT(editMultiplier()));
        break;
    case asn::EffectType::ChooseCard:
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, static_cast<int>(asn::PlaceType::SpecificPlace)));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editTarget2()), this, SLOT(editTarget2()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(editPlace2()), this, SLOT(editPlace2()));
        connect(qmlObject, SIGNAL(placeTypeChanged(int)), this, SLOT(onPlaceTypeChanged(int)));
        connect(qmlObject, SIGNAL(placeType2Changed(int)), this, SLOT(onPlaceType2Changed(int)));
        connect(qmlObject, SIGNAL(executorChanged(int)), this, SLOT(onPlayerChanged(int)));
        connect(qmlObject, SIGNAL(createSecondTarget()), this, SLOT(createSecondTarget()));
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
        connect(qmlObject, SIGNAL(valueTypeChanged(int)), this, SLOT(onValueTypeChanged(int)));
        connect(qmlObject, SIGNAL(editMultiplier()), this, SLOT(editMultiplier()));
        break;
    case asn::EffectType::MoveCard:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editFrom()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(editTo()), this, SLOT(editTo()));
        connect(qmlObject, SIGNAL(editTo2()), this, SLOT(editTo2()));
        connect(qmlObject, SIGNAL(addDestination()), this, SLOT(addDestination()));
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
    case asn::EffectType::ShotTriggerDamage:
    case asn::EffectType::DealDamage:
        QMetaObject::invokeMethod(qmlObject, "setValueInput", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(valueInputChanged(QString)), this, SLOT(onAttrChanged(QString)));
        connect(qmlObject, SIGNAL(damageTypeChanged(int)), this, SLOT(onValueTypeChanged(int)));
        connect(qmlObject, SIGNAL(editMultiplier()), this, SLOT(editMultiplier()));
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
    case asn::EffectType::DrawCard:
        QMetaObject::invokeMethod(qmlObject, "setNumValue", Q_ARG(QVariant, QString("1")));

        connect(qmlObject, SIGNAL(executorChanged(int)), this, SLOT(onPlayerChanged(int)));
        connect(qmlObject, SIGNAL(numModifierChanged(int)), this, SLOT(onNumModifierChanged(int)));
        connect(qmlObject, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
        break;
    case asn::EffectType::CannotAttack:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(attackTypeChanged(int)), this, SLOT(onAttackTypeChanged(int)));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::SideAttackWithoutPenalty:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::AddMarker:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(editDestination()), this, SLOT(editDestination()));
        connect(qmlObject, SIGNAL(faceOrientationChanged(int)), this, SLOT(onFaceOrientationChanged(int)));
        connect(qmlObject, SIGNAL(withMarkersChanged(bool)), this, SLOT(onWithMarkersChanged(bool)));

        QMetaObject::invokeMethod(qmlObject, "setFaceOrientation", Q_ARG(QVariant, QString("2")));
        break;
    case asn::EffectType::RemoveMarker:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editMarkerBearer()), this, SLOT(editMarkerBearer()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        break;
    case asn::EffectType::Shuffle:
        QMetaObject::invokeMethod(qmlObject, "setZone", Q_ARG(QVariant, QString("3")));

        connect(qmlObject, SIGNAL(playerChanged(int)), this, SLOT(onPlayerChanged(int)));
        connect(qmlObject, SIGNAL(zoneChanged(int)), this, SLOT(onZoneChanged(int)));
        break;
    case asn::EffectType::MoveWrToDeck:
        connect(qmlObject, SIGNAL(executorChanged(int)), this, SLOT(onPlayerChanged(int)));
        break;
    case asn::EffectType::CannotUseBackupOrEvent:
        connect(qmlObject, SIGNAL(backupOrEventChanged(int)), this, SLOT(onBackupOrEventChanged(int)));
        connect(qmlObject, SIGNAL(playerChanged(int)), this, SLOT(onPlayerChanged(int)));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::SwapCards:
        connect(qmlObject, SIGNAL(editChooseOne()), this, SLOT(editChooseOne()));
        connect(qmlObject, SIGNAL(editChooseTwo()), this, SLOT(editChooseTwo()));
        break;
    case asn::EffectType::OpponentAutoCannotDealDamage:
        QMetaObject::invokeMethod(qmlObject, "hideTarget");
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::PutOnStageRested:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(positionChanged(int)), this, SLOT(onPositionChanged(int)));
        break;
    case asn::EffectType::TriggerIconGain:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(triggerIconChanged(int)), this, SLOT(onTriggerIconChanged(int)));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::DelayedAbility:
        connect(qmlObject, SIGNAL(editAbilities()), this, SLOT(editAbilities()));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::CostSubstitution:
        connect(qmlObject, SIGNAL(editEffects()), this, SLOT(editEffectsField()));
        break;
    case asn::EffectType::CannotBecomeReversed:
    case asn::EffectType::CannotMove:
    case asn::EffectType::CannotBeChosen:
    case asn::EffectType::CannotStand:
    case asn::EffectType::CanPlayWithoutColorRequirement:
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::StockSwap:
        QMetaObject::invokeMethod(qmlObject, "setZone", Q_ARG(QVariant, QString("2")));
        connect(qmlObject, SIGNAL(zoneChanged(int)), this, SLOT(onZoneChanged(int)));
        break;
    case asn::EffectType::SkipPhase:
        QMetaObject::invokeMethod(qmlObject, "setPhase", Q_ARG(QVariant, QString("8")));
        connect(qmlObject, SIGNAL(phaseChanged(int)), this, SLOT(onPhaseChanged(int)));
        break;
    case asn::EffectType::ChooseTrait:
        QMetaObject::invokeMethod(qmlObject, "hideDuration");
        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        break;
    case asn::EffectType::TraitModification:
        QMetaObject::invokeMethod(qmlObject, "setType", Q_ARG(QVariant, static_cast<int>(asn::TraitModificationType::TraitGain)));
        QMetaObject::invokeMethod(qmlObject, "setTraitType", Q_ARG(QVariant, static_cast<int>(asn::TraitType::Value)));
        QMetaObject::invokeMethod(qmlObject, "setPlaceType", Q_ARG(QVariant, static_cast<int>(asn::PlaceType::SpecificPlace)));

        connect(qmlObject, SIGNAL(editTarget()), this, SLOT(editTarget()));
        connect(qmlObject, SIGNAL(editPlace()), this, SLOT(editPlace()));
        connect(qmlObject, SIGNAL(typeChanged(int)), this, SLOT(onTraitModificationTypeChanged(int)));
        connect(qmlObject, SIGNAL(traitTypeChanged(int)), this, SLOT(onTraitTypeChanged(int)));
        connect(qmlObject, SIGNAL(placeTypeChanged(int)), this, SLOT(onPlaceTypeChanged(int)));
        connect(qmlObject, SIGNAL(durationChanged(int)), this, SLOT(onDurationChanged(int)));
        break;
    case asn::EffectType::OtherEffect:
        connect(qmlObject, SIGNAL(cardCodeChanged(QString)), this, SLOT(cardCodeChanged(QString)));
        connect(qmlObject, SIGNAL(effectIdChanged(QString)), this, SLOT(onAttrChanged(QString)));
        break;
    default:
        break;
    }
}

void EffectImplComponent::createSecondTarget() {
    if (type == asn::EffectType::ChooseCard) {
        auto &e = std::get<asn::ChooseCard>(effect);
        if (e.targets.size() <= 1) {
            e.targets.push_back(defaultTargetAndPlace());
            emit componentChanged(effect);
        }
    }
}

void EffectImplComponent::editTarget(std::optional<asn::Target> target_) {
    auto &t = target_.has_value() ? *target_ : getTarget(effect, type);
    qmlTarget = std::make_unique<TargetComponent>(t, qmlObject);

    if (target_.has_value())
        connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &EffectImplComponent::secondTargetReady);
    else
        connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &EffectImplComponent::targetReady);

    connect(qmlTarget.get(), &TargetComponent::close, this, &EffectImplComponent::destroyTarget);
}

void EffectImplComponent::editTarget2() {
    auto target = getTarget2(effect, type);
    if (!target)
        return;

    qmlTarget = std::make_unique<TargetComponent>(target.value(), qmlObject);

    connect(qmlTarget.get(), &TargetComponent::componentChanged, this, &EffectImplComponent::target2Ready);

    connect(qmlTarget.get(), &TargetComponent::close, this, &EffectImplComponent::destroyTarget);
}

void EffectImplComponent::destroyTarget() {
    qmlTarget.reset();
}

void EffectImplComponent::targetReady(const asn::Target &t) {
    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto &e = std::get<asn::AttributeGain>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        e.targets[0].target = t;
        break;
    }
    case asn::EffectType::MoveCard: {
        auto &e = std::get<asn::MoveCard>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::AbilityGain: {
        auto &e = std::get<asn::AbilityGain>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::ChangeState: {
        auto &e = std::get<asn::ChangeState>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::CannotAttack: {
        auto &e = std::get<asn::CannotAttack>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::SideAttackWithoutPenalty: {
        auto &e = std::get<asn::SideAttackWithoutPenalty>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::AddMarker: {
        auto &e = std::get<asn::AddMarker>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::RemoveMarker: {
        auto &e = std::get<asn::RemoveMarker>(effect);
        e.targetMarker = t;
        break;
    }
    case asn::EffectType::CannotBecomeReversed: {
        auto &e = std::get<asn::CannotBecomeReversed>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::CannotMove: {
        auto &e = std::get<asn::CannotMove>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::PutOnStageRested: {
        auto &e = std::get<asn::PutOnStageRested>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::CannotStand: {
        auto &e = std::get<asn::CannotStand>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::CannotBeChosen: {
        auto &e = std::get<asn::CannotBeChosen>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::TriggerIconGain: {
        auto &e = std::get<asn::TriggerIconGain>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::CanPlayWithoutColorRequirement: {
        auto &e = std::get<asn::CanPlayWithoutColorRequirement>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::ChooseTrait: {
        auto &e = std::get<asn::ChooseTrait>(effect);
        e.target = t;
        break;
    }
    case asn::EffectType::TraitModification: {
        auto &e = std::get<asn::TraitModification>(effect);
        e.target.target = t;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::target2Ready(const asn::Target &t) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        if (e.targets.size() <= 1)
            return;
        e.targets[1].target = t;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::secondTargetReady(const asn::Target &t) {
    switch (type) {
    case asn::EffectType::AddMarker: {
        auto &e = std::get<asn::AddMarker>(effect);
        e.destination = t;
        break;
    }
    case asn::EffectType::RemoveMarker: {
        auto &e = std::get<asn::RemoveMarker>(effect);
        e.markerBearer = t;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::editMarkerBearer() {
    auto &e = std::get<asn::RemoveMarker>(effect);
    editTarget(e.markerBearer);
}

void EffectImplComponent::editDestination() {
    auto &e = std::get<asn::AddMarker>(effect);
    editTarget(e.destination);
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
    case asn::EffectType::ShotTriggerDamage:
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
    case asn::EffectType::OtherEffect: {
        auto &e = std::get<asn::OtherEffect>(effect);
        e.effectId = val;
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

void EffectImplComponent::onBackupOrEventChanged(int value) {
    switch (type) {
    case asn::EffectType::CannotUseBackupOrEvent: {
        auto &e = std::get<asn::CannotUseBackupOrEvent>(effect);
        e.what = static_cast<asn::BackupOrEvent>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onAttackTypeChanged(int value) {
    switch (type) {
    case asn::EffectType::CannotAttack: {
        auto &e = std::get<asn::CannotAttack>(effect);
        e.type = static_cast<asn::AttackType>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onFaceOrientationChanged(int value) {
    switch (type) {
    case asn::EffectType::AddMarker: {
        auto &e = std::get<asn::AddMarker>(effect);
        e.orientation = static_cast<asn::FaceOrientation>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::editSwapCards(int n) {
    auto &e = std::get<asn::SwapCards>(effect);
    const auto& eff = n == 1 ? e.first : e.second;
    qmlChooseCard = std::make_unique<ChooseCardComponent>(eff, qmlObject);

    if (n == 1)
        connect(qmlChooseCard.get(), &ChooseCardComponent::componentChanged, this, &EffectImplComponent::chooseOneReady);
    else
        connect(qmlChooseCard.get(), &ChooseCardComponent::componentChanged, this, &EffectImplComponent::chooseTwoReady);
    connect(qmlChooseCard.get(), &ChooseCardComponent::close, this, &EffectImplComponent::destroyChooseCard);
}

void EffectImplComponent::editChooseOne() {
    editSwapCards(1);
}

void EffectImplComponent::editChooseTwo() {
    editSwapCards(2);
}

void EffectImplComponent::destroyChooseCard() {
    qmlChooseCard.reset();
}

void EffectImplComponent::chooseOneReady(const asn::ChooseCard &e) {
    auto &eff = std::get<asn::SwapCards>(effect);
    eff.first = e;
    emit componentChanged(effect);
}

void EffectImplComponent::chooseTwoReady(const asn::ChooseCard &e) {
    auto &eff = std::get<asn::SwapCards>(effect);
    eff.second = e;
    emit componentChanged(effect);
}

void EffectImplComponent::onTriggerIconChanged(int value) {
    auto &eff = std::get<asn::TriggerIconGain>(effect);
    eff.triggerIcon = static_cast<asn::TriggerIcon>(value);
    emit componentChanged(effect);
}

void EffectImplComponent::onWithMarkersChanged(bool value)
{
    switch (type) {
    case asn::EffectType::AddMarker: {
        auto &e = std::get<asn::AddMarker>(effect);
        e.withMarkers = value;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onTraitModificationTypeChanged(int value) {
    switch (type) {
    case asn::EffectType::TraitModification: {
        auto &e = std::get<asn::TraitModification>(effect);
        e.type = static_cast<asn::TraitModificationType>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onTraitTypeChanged(int value) {
    switch (type) {
    case asn::EffectType::TraitModification: {
        auto &e = std::get<asn::TraitModification>(effect);
        e.traitType = static_cast<asn::TraitType>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::cardCodeChanged(QString code) {
    switch (type) {
    case asn::EffectType::OtherEffect: {
        auto &e = std::get<asn::OtherEffect>(effect);
        e.cardCode = code.toStdString();
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
    case asn::EffectType::CannotAttack: {
        auto &e = std::get<asn::CannotAttack>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::SideAttackWithoutPenalty: {
        auto &e = std::get<asn::SideAttackWithoutPenalty>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::CannotUseBackupOrEvent: {
        auto &e = std::get<asn::CannotUseBackupOrEvent>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::OpponentAutoCannotDealDamage: {
        auto &e = std::get<asn::OpponentAutoCannotDealDamage>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::CannotBecomeReversed: {
        auto &e = std::get<asn::CannotBecomeReversed>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::CannotMove: {
        auto &e = std::get<asn::CannotMove>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::CannotStand: {
        auto &e = std::get<asn::CannotStand>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::CannotBeChosen: {
        auto &e = std::get<asn::CannotBeChosen>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::TriggerIconGain: {
        auto &e = std::get<asn::TriggerIconGain>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::CanPlayWithoutColorRequirement: {
        auto &e = std::get<asn::CanPlayWithoutColorRequirement>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::DelayedAbility: {
        auto &e = std::get<asn::DelayedAbility>(effect);
        e.duration = value;
        break;
    }
    case asn::EffectType::TraitModification: {
        auto &e = std::get<asn::TraitModification>(effect);
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

void EffectImplComponent::editPlace2() {
    auto place = getPlace2(effect, type);
    if (!place)
        return;
    qmlPlace = std::make_unique<PlaceComponent>(place.value(), qmlObject);

    connect(qmlPlace.get(), &PlaceComponent::componentChanged, this, &EffectImplComponent::place2Ready);
    connect(qmlPlace.get(), &PlaceComponent::close, this, &EffectImplComponent::destroyPlace);
}

void EffectImplComponent::editTo() {
    auto &e = std::get<asn::MoveCard>(effect);
    editPlace(e.to[0]);
    currentCardIndex = 0;
}

void EffectImplComponent::editTo2() {
    auto &e = std::get<asn::MoveCard>(effect);
    editPlace(e.to[1]);
    currentCardIndex = 1;
}

void EffectImplComponent::addDestination() {
    auto &e = std::get<asn::MoveCard>(effect);
    e.to.push_back(defaultPlace);
    emit componentChanged(effect);
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
    case asn::EffectType::AddMarker: {
        auto &e = std::get<asn::AddMarker>(effect);
        e.from = p;
        break;
    }
    case asn::EffectType::RemoveMarker: {
        auto &e = std::get<asn::RemoveMarker>(effect);
        e.place = p;
        break;
    }
    case asn::EffectType::PutOnStageRested: {
        auto &e = std::get<asn::PutOnStageRested>(effect);
        e.from = p;
        break;
    }
    case asn::EffectType::TraitModification: {
        auto &e = std::get<asn::TraitModification>(effect);
        e.target.place = p;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::place2Ready(const asn::Place &p) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        if (e.targets.size() <= 1)
            return;
        e.targets[1].place = p;
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::placeToReady(const asn::Place &p) {
    auto &e = std::get<asn::MoveCard>(effect);
    e.to[currentCardIndex] = p;
    emit componentChanged(effect);
}

void EffectImplComponent::onPlaceTypeChanged(int value) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        e.targets[0].placeType = static_cast<asn::PlaceType>(value);
        if (e.targets[0].placeType != asn::PlaceType::SpecificPlace) {
            e.targets[0].place = std::nullopt;
        } else {
            auto defaultPlace = asn::Place();
            defaultPlace.owner = asn::Player::Player;
            defaultPlace.pos = asn::Position::NotSpecified;
            defaultPlace.zone = asn::Zone::Stage;
            e.targets[0].place = defaultPlace;
        }
        break;
    }
    case asn::EffectType::TraitModification: {
        auto &e = std::get<asn::TraitModification>(effect);
        e.target.placeType = static_cast<asn::PlaceType>(value);
        if (e.target.placeType != asn::PlaceType::SpecificPlace) {
            e.target.place = std::nullopt;
        } else {
            auto defaultPlace = asn::Place();
            defaultPlace.owner = asn::Player::Player;
            defaultPlace.pos = asn::Position::NotSpecified;
            defaultPlace.zone = asn::Zone::Stage;
            e.target.place = defaultPlace;
        }
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onPlaceType2Changed(int value) {
    switch (type) {
    case asn::EffectType::ChooseCard: {
        auto &e = std::get<asn::ChooseCard>(effect);
        if (e.targets.size() <= 1)
            return;
        e.targets[1].placeType = static_cast<asn::PlaceType>(value);
        if (e.targets[1].placeType == asn::PlaceType::Selection)
            e.targets[1].place = std::nullopt;
        else {
            auto defaultPlace = asn::Place();
            defaultPlace.owner = asn::Player::Player;
            defaultPlace.pos = asn::Position::NotSpecified;
            defaultPlace.zone = asn::Zone::Stage;
            e.targets[1].place = defaultPlace;
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
    case asn::EffectType::DrawCard: {
        auto &e = std::get<asn::DrawCard>(effect);
        e.executor = static_cast<asn::Player>(value);
        break;
    }
    case asn::EffectType::Shuffle: {
        auto &e = std::get<asn::Shuffle>(effect);
        e.owner = static_cast<asn::Player>(value);
        break;
    }
    case asn::EffectType::MoveWrToDeck: {
        auto &e = std::get<asn::MoveWrToDeck>(effect);
        e.executor = static_cast<asn::Player>(value);
        break;
    }
    case asn::EffectType::CannotUseBackupOrEvent: {
        auto &e = std::get<asn::CannotUseBackupOrEvent>(effect);
        e.player = static_cast<asn::Player>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onZoneChanged(int value) {
    switch (type) {
    case asn::EffectType::Shuffle: {
        auto &e = std::get<asn::Shuffle>(effect);
        e.zone = static_cast<asn::Zone>(value);
        break;
    }
    case asn::EffectType::StockSwap: {
        auto &e = std::get<asn::StockSwap>(effect);
        e.zone = static_cast<asn::Zone>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onPositionChanged(int value) {
    switch (type) {
    case asn::EffectType::PutOnStageRested: {
        auto &e = std::get<asn::PutOnStageRested>(effect);
        e.to = static_cast<asn::Position>(value);
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::onPhaseChanged(int value) {
    switch (type) {
    case asn::EffectType::SkipPhase: {
        auto &e = std::get<asn::SkipPhase>(effect);
        e.skipUntil = static_cast<asn::Phase>(value);
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
    case asn::EffectType::DrawCard: {
        auto &e = std::get<asn::DrawCard>(effect);
        e.value.mod = static_cast<asn::NumModifier>(value);
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
    case asn::EffectType::DrawCard: {
        auto &e = std::get<asn::DrawCard>(effect);
        e.value.value = val;
        break;
    }
    default:
        assert(false);
    }

    emit componentChanged(effect);
}

void EffectImplComponent::onValueTypeChanged(int value) {
    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto &e = std::get<asn::AttributeGain>(effect);
        e.gainType = static_cast<asn::ValueType>(value);
        if (e.gainType == asn::ValueType::Multiplier) {
            if (!e.modifier)
                e.modifier = getDefaultMultiplier();
        } else {
            e.modifier.reset();
        }
        break;
    }
    case asn::EffectType::Look: {
        auto &e = std::get<asn::Look>(effect);
        e.valueType = static_cast<asn::ValueType>(value);
        if (e.valueType == asn::ValueType::Multiplier) {
            if (!e.multiplier)
                e.multiplier = getDefaultMultiplier();
        } else {
            e.multiplier.reset();
        }
        break;
    }
    case asn::EffectType::ShotTriggerDamage:
    case asn::EffectType::DealDamage: {
        auto &e = std::get<asn::DealDamage>(effect);
        e.damageType = static_cast<asn::ValueType>(value);
        if (e.damageType == asn::ValueType::Multiplier) {
            if (!e.modifier)
                e.modifier = getDefaultMultiplier();
        } else {
            e.modifier.reset();
        }
        break;
    }
    default:
        assert(false);
    }
    emit componentChanged(effect);
}

void EffectImplComponent::editMultiplier() {
    const auto &m = getMultiplier(effect, type);
    qmlMultiplier = std::make_unique<MultiplierComponent>(*m, qmlObject);

    connect(qmlMultiplier.get(), &MultiplierComponent::componentChanged, this, &EffectImplComponent::multiplierReady);
    connect(qmlMultiplier.get(), &MultiplierComponent::close, this, &EffectImplComponent::destroyMultiplier);
}

void EffectImplComponent::destroyMultiplier() {
    qmlMultiplier.reset();
}

void EffectImplComponent::multiplierReady(const asn::Multiplier &m) {
    switch (type) {
    case asn::EffectType::AttributeGain: {
        auto &e = std::get<asn::AttributeGain>(effect);
        e.modifier = m;
        break;
    }
    case asn::EffectType::Look: {
        auto &e = std::get<asn::Look>(effect);
        e.multiplier = m;
        break;
    }
    case asn::EffectType::ShotTriggerDamage:
    case asn::EffectType::DealDamage: {
        auto &e = std::get<asn::DealDamage>(effect);
        e.modifier = m;
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
    std::vector<asn::Effect> stubEffects;
    std::vector<asn::Effect> *eff = &stubEffects;
    switch (type) {
    case asn::EffectType::NonMandatory:
        eff = &std::get<asn::NonMandatory>(effect).effect;
        break;
    case asn::EffectType::FlipOver:
        eff = &std::get<asn::FlipOver>(effect).effect;
        break;
    case asn::EffectType::CostSubstitution: {
        const auto &eff = std::get<asn::CostSubstitution>(effect);
        if (eff.effect)
            stubEffects.push_back(*eff.effect);
        break;
    }
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
    case asn::EffectType::CostSubstitution: {
        auto &e = std::get<asn::CostSubstitution>(effect);
        if (effects.size() > 0)
            e.effect = std::make_shared<asn::Effect>(effects[0]);
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
    case asn::EffectType::DelayedAbility: {
        asn::Ability ability;
        ability.type = asn::AbilityType::Auto;
        ability.ability = *std::get<asn::DelayedAbility>(effect).ability;
        abilities.push_back(ability);
    }
    default: break;
    }

    qmlAbilities = std::make_unique<ArrayOfAbilitiesComponent>(abilities, qmlObject);
    if (type == asn::EffectType::PerformEffect)
        qmlAbilities->fixEventAbility();
    if (type == asn::EffectType::DelayedAbility)
        qmlAbilities->fixAutoAbility();

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
    } else if (type == asn::EffectType::DelayedAbility) {
        auto &e = std::get<asn::DelayedAbility>(effect);
        *e.ability = std::get<asn::AutoAbility>(a[0].ability);
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
