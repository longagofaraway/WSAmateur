#include "decode.h"

using namespace asn;

AttributeGain decodeAttributeGain(Iterator &it, Iterator end) {
    AttributeGain e;
    e.target = decodeTarget(it, end);
    e.type = decodeEnum<AttributeType>(it, end);
    e.gainType = decodeEnum<ValueType>(it, end);
    e.value = decodeInt32(it, end);
    if (e.gainType == ValueType::Multiplier)
        e.modifier = decodeMultiplier(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

TargetAndPlace decodeTargetAndPlace(Iterator &it, Iterator end) {
    TargetAndPlace e;
    e.target = decodeTarget(it, end);
    e.placeType = decodeEnum<PlaceType>(it, end);
    if (e.placeType == PlaceType::SpecificPlace)
        e.place = decodePlace(it, end);
    return e;
}

ChooseCard decodeChooseCard(Iterator &it, Iterator end) {
    ChooseCard e;
    e.executor = decodeEnum<Player>(it, end);
    e.targets = decodeArray(it, end, decodeTargetAndPlace);
    e.excluding = decodeArray(it, end, decodeCard);
    return e;
}

RevealCard decodeRevealCard(Iterator &it, Iterator end) {
    RevealCard e;
    e.type = decodeEnum<RevealType>(it, end);
    e.number = decodeNumber(it, end);
    if (e.type == RevealType::FromHand)
        e.card = decodeCard(it, end);
    return e;
}

MoveCard decodeMoveCard(Iterator &it, Iterator end) {
    MoveCard e;
    e.executor = decodeEnum<Player>(it, end);
    e.target = decodeTarget(it, end);
    e.from = decodePlace(it, end);
    e.to = decodeArray(it, end, decodePlace);
    e.order = decodeEnum<Order>(it, end);
    return e;
}

SearchTarget decodeSearchTarget(Iterator &it, Iterator end) {
    SearchTarget s;
    s.number = decodeNumber(it, end);
    s.cards = decodeArray(it, end, decodeCard);
    return s;
}

SearchCard decodeSearchCard(Iterator &it, Iterator end) {
    SearchCard e;
    e.targets = decodeArray(it, end, decodeSearchTarget);
    e.place = decodePlace(it, end);
    return e;
}

PayCost decodePayCost(Iterator &it, Iterator end) {
    PayCost e;
    e.ifYouDo = decodeArray(it, end, decodeEffect);
    e.ifYouDont = decodeArray(it, end, decodeEffect);
    return e;
}

AbilityGain decodeAbilityGain(Iterator &it, Iterator end) {
    AbilityGain e;
    e.target = decodeTarget(it, end);
    e.number = decodeUInt8(it, end);
    e.abilities = decodeArray(it, end, decodeAbility);
    e.duration = decodeUInt8(it, end);
    return e;
}

PerformEffect decodePerformEffect(Iterator &it, Iterator end) {
    PerformEffect e;
    e.numberOfEffects = decodeUInt8(it, end);
    e.numberOfTimes = decodeUInt8(it, end);
    e.effects = decodeArray(it, end, decodeEventAbility);
    return e;
}

FlipOver decodeFlipOver(Iterator &it, Iterator end) {
    FlipOver e;
    e.number = decodeNumber(it, end);
    e.forEach = decodeCard(it, end);
    e.effect = decodeArray(it, end, decodeEffect);
    return e;
}

NonMandatory decodeNonMandatory(Iterator &it, Iterator end) {
    NonMandatory e;
    e.effect = decodeArray(it, end, decodeEffect);
    e.ifYouDo = decodeArray(it, end, decodeEffect);
    e.ifYouDont = decodeArray(it, end, decodeEffect);
    return e;
}

ChangeState decodeChangeState(Iterator &it, Iterator end) {
    ChangeState e;
    e.target = decodeTarget(it, end);
    e.state = decodeEnum<State>(it, end);
    return e;
}

DealDamage decodeDealDamage(Iterator &it, Iterator end) {
    DealDamage e;
    e.damageType = decodeEnum<ValueType>(it, end);
    e.damage = decodeUInt8(it, end);
    if (e.damageType == ValueType::Multiplier)
        e.modifier = decodeMultiplier(it, end);
    return e;
}

CannotUseBackupOrEvent decodeCannotUseBackupOrEvent(Iterator &it, Iterator end) {
    CannotUseBackupOrEvent e;
    e.what = decodeEnum<BackupOrEvent>(it, end);
    e.player = decodeEnum<Player>(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

SwapCards decodeSwapCards(Iterator &it, Iterator end) {
    SwapCards e;
    e.first = decodeChooseCard(it, end);
    e.second = decodeChooseCard(it, end);
    return e;
}

CannotAttack decodeCannotAttack(Iterator &it, Iterator end) {
    CannotAttack e;
    e.target = decodeTarget(it, end);
    e.type = decodeEnum<AttackType>(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

AddMarker decodeAddMarker(Iterator &it, Iterator end) {
    AddMarker e;
    e.target = decodeTarget(it, end);
    e.from = decodePlace(it, end);
    e.destination = decodeTarget(it, end);
    e.orientation = decodeEnum<FaceOrientation>(it, end);
    e.withMarkers = decodeBool(it, end);
    return e;
}

Replay decodeReplay(Iterator &it, Iterator end) {
    Replay e;
    e.name = decodeString(it, end);
    e.effects = decodeArray(it, end, decodeEffect);
    return e;
}

Look decodeLook(Iterator &it, Iterator end) {
    Look e;
    e.number = decodeNumber(it, end);
    e.place = decodePlace(it, end);
    e.valueType = decodeEnum<ValueType>(it, end);
    if (e.valueType == ValueType::Multiplier)
        e.multiplier = decodeMultiplier(it, end);
    return e;
}

DrawCard decodeDrawCard(Iterator &it, Iterator end) {
    DrawCard e;
    e.executor = decodeEnum<Player>(it, end);
    e.value = decodeNumber(it, end);
    return e;
}

Shuffle decodeShuffle(Iterator &it, Iterator end) {
    Shuffle e;
    e.zone = decodeEnum<Zone>(it, end);
    e.owner = decodeEnum<Player>(it, end);
    return e;
}

CannotBecomeReversed decodeCannotBecomeReversed(Iterator &it, Iterator end) {
    CannotBecomeReversed e;
    e.target = decodeTarget(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

OpponentAutoCannotDealDamage decodeOpponentAutoCannotDealDamage(Iterator &it, Iterator end) {
    OpponentAutoCannotDealDamage e;
    e.duration = decodeUInt8(it, end);
    return e;
}

CannotMove decodeCannotMove(Iterator &it, Iterator end) {
    CannotMove e;
    e.target = decodeTarget(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

SideAttackWithoutPenalty decodeSideAttackWithoutPenalty(Iterator &it, Iterator end) {
    SideAttackWithoutPenalty e;
    e.target = decodeTarget(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

PutOnStageRested decodePutOnStageRested(Iterator &it, Iterator end) {
    PutOnStageRested e;
    e.target = decodeTarget(it, end);
    e.from = decodePlace(it, end);
    e.to = decodeEnum<Position>(it, end);
    return e;
}

RemoveMarker decodeRemoveMarker(Iterator &it, Iterator end) {
    RemoveMarker e;
    e.targetMarker = decodeTarget(it, end);
    e.markerBearer = decodeTarget(it, end);
    e.place = decodePlace(it, end);
    return e;
}

CannotStand decodeCannotStand(Iterator &it, Iterator end) {
    CannotStand e;
    e.target = decodeTarget(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

CannotBeChosen decodeCannotBeChosen(Iterator &it, Iterator end) {
    CannotBeChosen e;
    e.target = decodeTarget(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

TriggerIconGain decodeTriggerIconGain(Iterator &it, Iterator end) {
    TriggerIconGain e;
    e.target = decodeTarget(it, end);
    e.triggerIcon = decodeEnum<TriggerIcon>(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

CanPlayWithoutColorRequirement decodeCanPlayWithoutColorRequirement(Iterator &it, Iterator end) {
    CanPlayWithoutColorRequirement e;
    e.target = decodeTarget(it, end);
    e.duration = decodeUInt8(it, end);
    return e;
}

DelayedAbility decodeDelayedAbility(Iterator &it, Iterator end) {
    DelayedAbility e;
    e.ability = std::make_shared<AutoAbility>(decodeAutoAbility(it, end));
    e.duration = decodeUInt8(it, end);
    return e;
}

CostSubstitution decodeCostSubstitution(Iterator &it, Iterator end) {
    CostSubstitution e;
    e.effect = std::make_shared<Effect>(decodeEffect(it, end));
    return e;
}

StockSwap decodeStockSwap(Iterator &it, Iterator end) {
    StockSwap e;
    e.zone = decodeEnum<Zone>(it, end);
    return e;
}

SkipPhase decodeSkipPhase(Iterator &it, Iterator end) {
    SkipPhase e;
    e.skipUntil = decodeEnum<Phase>(it, end);
    return e;
}

OtherEffect decodeOtherEffect(Iterator &it, Iterator end) {
    OtherEffect e;
    e.cardCode = decodeString(it, end);
    e.effectId = decodeInt8(it, end);
    return e;
}

Effect decodeEffect(Iterator &it, Iterator end) {
    Effect e;

    e.type = decodeEnum<EffectType>(it, end);
    e.cond = decodeCondition(it, end);
    switch (e.type) {
    case EffectType::AttributeGain:
        e.effect = decodeAttributeGain(it, end);
        break;
    case EffectType::ChooseCard:
        e.effect = decodeChooseCard(it, end);
        break;
    case EffectType::RevealCard:
        e.effect = decodeRevealCard(it, end);
        break;
    case EffectType::MoveCard:
        e.effect = decodeMoveCard(it, end);
        break;
    case EffectType::SearchCard:
        e.effect = decodeSearchCard(it, end);
        break;
    case EffectType::PayCost:
        e.effect = decodePayCost(it, end);
        break;
    case EffectType::AbilityGain:
        e.effect = decodeAbilityGain(it, end);
        break;
    case EffectType::PerformEffect:
        e.effect = decodePerformEffect(it, end);
        break;
    case EffectType::MoveWrToDeck:
        e.effect = MoveWrToDeck{ decodeEnum<Player>(it, end) };
        break;
    case EffectType::FlipOver:
        e.effect = decodeFlipOver(it, end);
        break;
    case EffectType::Backup:
        e.effect = Backup{ decodeInt32(it, end), decodeInt8(it, end) };
        break;
    case EffectType::NonMandatory:
        e.effect = decodeNonMandatory(it, end);
        break;
    case EffectType::ChangeState:
        e.effect = decodeChangeState(it, end);
        break;
    case EffectType::ShotTriggerDamage:
    case EffectType::DealDamage:
        e.effect = decodeDealDamage(it, end);
        break;
    case EffectType::CannotUseBackupOrEvent:
        e.effect = decodeCannotUseBackupOrEvent(it, end);
        break;
    case EffectType::SwapCards:
        e.effect = decodeSwapCards(it, end);
        break;
    case EffectType::CannotAttack:
        e.effect = decodeCannotAttack(it, end);
        break;
    case EffectType::AddMarker:
        e.effect = decodeAddMarker(it, end);
        break;
    case EffectType::Bond:
        e.effect = Bond{ decodeString(it, end) };
        break;
    case EffectType::PerformReplay:
        e.effect = PerformReplay{ decodeString(it, end) };
        break;
    case EffectType::Replay:
        e.effect = decodeReplay(it, end);
        break;
    case EffectType::DrawCard:
        e.effect = decodeDrawCard(it, end);
        break;
    case EffectType::Look:
        e.effect = decodeLook(it, end);
        break;
    case EffectType::Shuffle:
        e.effect = decodeShuffle(it, end);
        break;
    case EffectType::CannotBecomeReversed:
        e.effect = decodeCannotBecomeReversed(it, end);
        break;
    case EffectType::OpponentAutoCannotDealDamage:
        e.effect = decodeOpponentAutoCannotDealDamage(it, end);
        break;
    case EffectType::CannotMove:
        e.effect = decodeCannotMove(it, end);
        break;
    case EffectType::SideAttackWithoutPenalty:
        e.effect = decodeSideAttackWithoutPenalty(it, end);
        break;
    case EffectType::OtherEffect:
        e.effect = decodeOtherEffect(it, end);
        break;
    case EffectType::PutOnStageRested:
        e.effect = decodePutOnStageRested(it, end);
        break;
    case EffectType::RemoveMarker:
        e.effect = decodeRemoveMarker(it, end);
        break;
    case EffectType::CannotStand:
        e.effect = decodeCannotStand(it, end);
        break;
    case EffectType::CannotBeChosen:
        e.effect = decodeCannotBeChosen(it, end);
        break;
    case EffectType::CanPlayWithoutColorRequirement:
        e.effect = decodeCanPlayWithoutColorRequirement(it, end);
        break;
    case EffectType::TriggerIconGain:
        e.effect = decodeTriggerIconGain(it, end);
        break;
    case EffectType::DelayedAbility:
        e.effect = decodeDelayedAbility(it, end);
        break;
    case EffectType::CostSubstitution:
        e.effect = decodeCostSubstitution(it, end);
        break;
    case EffectType::StockSwap:
        e.effect = decodeStockSwap(it, end);
        break;
    case EffectType::SkipPhase:
        e.effect = decodeSkipPhase(it, end);
        break;
    default:
        break;
    }

    return e;
}
