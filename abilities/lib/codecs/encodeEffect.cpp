#include "encode.h"

#include "encDecUtils.h"

using namespace asn;

void encodeAttributeGain(const AttributeGain &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.type));
    buf.push_back(static_cast<uint8_t>(e.gainType));
    toBufLE(zzenc_32(e.value), buf);
    if (e.modifier)
        encodeMultiplier(*e.modifier, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeTargetAndPlace(const TargetAndPlace &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.placeType));
    if (e.placeType == PlaceType::SpecificPlace)
        encodePlace(*e.place, buf);
}

void encodeChooseCard(const ChooseCard &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.executor));
    encodeArray(e.targets, buf, encodeTargetAndPlace);
    encodeArray(e.excluding, buf, encodeCard);
}

void encodeRevealCard(const RevealCard &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.type));
    encodeNumber(e.number, buf);
    if (e.card)
        encodeCard(*e.card, buf);
}

void encodeMoveCard(const MoveCard &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.executor));
    encodeTarget(e.target, buf);
    encodePlace(e.from, buf);
    encodeArray(e.to, buf, encodePlace);
    buf.push_back(static_cast<uint8_t>(e.order));
}

void encodeSearchTarget(const SearchTarget &e, Buf &buf) {
    encodeNumber(e.number, buf);
    encodeArray(e.cards, buf, encodeCard);
}

void encodeSearchCard(const SearchCard &e, Buf &buf) {
    encodeArray(e.targets, buf, encodeSearchTarget);
    encodePlace(e.place, buf);
}

void encodePayCost(const PayCost &e, Buf &buf) {
    encodeArray(e.ifYouDo, buf, encodeEffect);
    encodeArray(e.ifYouDont, buf, encodeEffect);
}

void encodeAbilityGain(const AbilityGain &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.number));
    encodeArray(e.abilities, buf, encodeAbility);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodePerformEffect(const PerformEffect &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.numberOfEffects));
    buf.push_back(static_cast<uint8_t>(e.numberOfTimes));
    encodeArray(e.effects, buf, encodeEventAbility);
}

void encodeFlipOver(const FlipOver &e, Buf &buf) {
    encodeNumber(e.number, buf);
    encodeCard(e.forEach, buf);
    encodeArray(e.effect, buf, encodeEffect);
}

void encodeNonMandatory(const NonMandatory &e, Buf &buf) {
    encodeArray(e.effect, buf, encodeEffect);
    encodeArray(e.ifYouDo, buf, encodeEffect);
    encodeArray(e.ifYouDont, buf, encodeEffect);
}

void encodeChangeState(const ChangeState &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.state));
}

void encodeDealDamage(const DealDamage &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.damageType));
    buf.push_back(static_cast<uint8_t>(e.damage));
    if (e.modifier)
        encodeMultiplier(*e.modifier, buf);
}

void encodeCannotUseBackupOrEvent(const CannotUseBackupOrEvent &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.what));
    buf.push_back(static_cast<uint8_t>(e.player));
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeSwapCards(const SwapCards &e, Buf &buf) {
    encodeChooseCard(e.first, buf);
    encodeChooseCard(e.second, buf);
}

void encodeCannotAttack(const CannotAttack &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.type));
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeAddMarker(const AddMarker &e, Buf &buf) {
    encodeTarget(e.target, buf);
    encodePlace(e.from, buf);
    encodeTarget(e.destination, buf);
    buf.push_back(static_cast<uint8_t>(e.orientation));
    buf.push_back(e.withMarkers ? 1 : 0);
}

void encodeReplay(const Replay &e, Buf &buf) {
    encodeString(e.name, buf);
    encodeArray(e.effects, buf, encodeEffect);
}

void encodeLook(const Look &e, Buf &buf) {
    encodeNumber(e.number, buf);
    encodePlace(e.place, buf);
    buf.push_back(static_cast<uint8_t>(e.valueType));
    if (e.valueType == ValueType::Multiplier)
        encodeMultiplier(e.multiplier.value(), buf);
}

void encodeDrawCard(const DrawCard &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.executor));
    encodeNumber(e.value, buf);
}

void encodeShuffle(const Shuffle &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.zone));
    buf.push_back(static_cast<uint8_t>(e.owner));
}

void encodeBackup(const Backup &e, Buf &buf) {
    toBufLE(zzenc_32(e.power), buf);
    buf.push_back(zzenc_8(e.level));
}

void encodeCannotBecomeReversed(const CannotBecomeReversed &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeOpponentAutoCannotDealDamage(const OpponentAutoCannotDealDamage &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeCannotMove(const CannotMove &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeSideAttackWithoutPenalty(const SideAttackWithoutPenalty &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodePutOnStageRested(const PutOnStageRested &e, Buf &buf) {
    encodeTarget(e.target, buf);
    encodePlace(e.from, buf);
    buf.push_back(static_cast<uint8_t>(e.to));
}

void encodeRemoveMarker(const RemoveMarker &e, Buf &buf) {
    encodeTarget(e.targetMarker, buf);
    encodeTarget(e.markerBearer, buf);
    encodePlace(e.place, buf);
}

void encodeCannotStand(const CannotStand &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeCannotBeChosen(const CannotBeChosen &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeTriggerIconGain(const TriggerIconGain &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.triggerIcon));
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeCanPlayWithoutColorRequirement(const CanPlayWithoutColorRequirement &e, Buf &buf) {
    encodeTarget(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeDelayedAbility(const DelayedAbility &e, Buf &buf) {
    encodeAutoAbility(*e.ability, buf);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeCostSubstitution(const CostSubstitution &e, Buf &buf) {
    encodeEffect(*e.effect, buf);
}

void encodeSkipPhase(const SkipPhase &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.skipUntil));
}

void encodeStockSwap(const StockSwap &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.zone));
}

void encodeChooseTrait(const ChooseTrait &e, Buf &buf) {
    encodeTargetAndPlace(e.target, buf);
}

void encodeTraitModification(const TraitModification &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.type));
    encodeTargetAndPlace(e.target, buf);
    buf.push_back(static_cast<uint8_t>(e.traitType));
    encodeArray(e.traits, buf, encodeString);
    buf.push_back(static_cast<uint8_t>(e.duration));
}

void encodeOtherEffect(const OtherEffect &e, Buf &buf) {
    encodeString(e.cardCode, buf);
    buf.push_back(zzenc_8(e.effectId));
}

void encodeEffect(const Effect &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.type));
    encodeCondition(e.cond, buf);

    switch (e.type) {
    case EffectType::AttributeGain:
        encodeAttributeGain(std::get<AttributeGain>(e.effect), buf);
        break;
    case EffectType::ChooseCard:
        encodeChooseCard(std::get<ChooseCard>(e.effect), buf);
        break;
    case EffectType::RevealCard:
        encodeRevealCard(std::get<RevealCard>(e.effect), buf);
        break;
    case EffectType::MoveCard:
        encodeMoveCard(std::get<MoveCard>(e.effect), buf);
        break;
    case EffectType::SearchCard:
        encodeSearchCard(std::get<SearchCard>(e.effect), buf);
        break;
    case EffectType::PayCost:
        encodePayCost(std::get<PayCost>(e.effect), buf);
        break;
    case EffectType::AbilityGain:
        encodeAbilityGain(std::get<AbilityGain>(e.effect), buf);
        break;
    case EffectType::PerformEffect:
        encodePerformEffect(std::get<PerformEffect>(e.effect), buf);
        break;
    case EffectType::MoveWrToDeck:
        buf.push_back(static_cast<uint8_t>(std::get<MoveWrToDeck>(e.effect).executor));
        break;
    case EffectType::FlipOver:
        encodeFlipOver(std::get<FlipOver>(e.effect), buf);
        break;
    case EffectType::Backup:
        encodeBackup(std::get<Backup>(e.effect), buf);
        break;
    case EffectType::NonMandatory:
        encodeNonMandatory(std::get<NonMandatory>(e.effect), buf);
        break;
    case EffectType::ChangeState:
        encodeChangeState(std::get<ChangeState>(e.effect), buf);
        break;
    case EffectType::ShotTriggerDamage:
    case EffectType::DealDamage:
        encodeDealDamage(std::get<DealDamage>(e.effect), buf);
        break;
    case EffectType::CannotUseBackupOrEvent:
        encodeCannotUseBackupOrEvent(std::get<CannotUseBackupOrEvent>(e.effect), buf);
        break;
    case EffectType::SwapCards:
        encodeSwapCards(std::get<SwapCards>(e.effect), buf);
        break;
    case EffectType::CannotAttack:
        encodeCannotAttack(std::get<CannotAttack>(e.effect), buf);
        break;
    case EffectType::AddMarker:
        encodeAddMarker(std::get<AddMarker>(e.effect), buf);
        break;
    case EffectType::Bond:
        encodeString(std::get<Bond>(e.effect).value, buf);
        break;
    case EffectType::PerformReplay:
        encodeString(std::get<PerformReplay>(e.effect).value, buf);
        break;
    case EffectType::Replay:
        encodeReplay(std::get<Replay>(e.effect), buf);
        break;
    case EffectType::DrawCard:
        encodeDrawCard(std::get<DrawCard>(e.effect), buf);
        break;
    case EffectType::Look:
        encodeLook(std::get<Look>(e.effect), buf);
        break;
    case EffectType::Shuffle:
        encodeShuffle(std::get<Shuffle>(e.effect), buf);
        break;
    case EffectType::CannotBecomeReversed:
        encodeCannotBecomeReversed(std::get<CannotBecomeReversed>(e.effect), buf);
        break;
    case EffectType::OpponentAutoCannotDealDamage:
        encodeOpponentAutoCannotDealDamage(std::get<OpponentAutoCannotDealDamage>(e.effect), buf);
        break;
    case EffectType::CannotMove:
        encodeCannotMove(std::get<CannotMove>(e.effect), buf);
        break;
    case EffectType::SideAttackWithoutPenalty:
        encodeSideAttackWithoutPenalty(std::get<SideAttackWithoutPenalty>(e.effect), buf);
        break;
    case EffectType::PutOnStageRested:
        encodePutOnStageRested(std::get<PutOnStageRested>(e.effect), buf);
        break;
    case EffectType::RemoveMarker:
        encodeRemoveMarker(std::get<RemoveMarker>(e.effect), buf);
        break;
    case EffectType::CannotStand:
        encodeCannotStand(std::get<CannotStand>(e.effect), buf);
        break;
    case EffectType::CannotBeChosen:
        encodeCannotBeChosen(std::get<CannotBeChosen>(e.effect), buf);
        break;
    case EffectType::TriggerIconGain:
        encodeTriggerIconGain(std::get<TriggerIconGain>(e.effect), buf);
        break;
    case EffectType::CanPlayWithoutColorRequirement:
        encodeCanPlayWithoutColorRequirement(std::get<CanPlayWithoutColorRequirement>(e.effect), buf);
        break;
    case EffectType::DelayedAbility:
        encodeDelayedAbility(std::get<DelayedAbility>(e.effect), buf);
        break;
    case EffectType::CostSubstitution:
        encodeCostSubstitution(std::get<CostSubstitution>(e.effect), buf);
        break;
    case EffectType::StockSwap:
        encodeStockSwap(std::get<StockSwap>(e.effect), buf);
        break;
    case EffectType::SkipPhase:
        encodeSkipPhase(std::get<SkipPhase>(e.effect), buf);
        break;
    case EffectType::ChooseTrait:
        encodeChooseTrait(std::get<ChooseTrait>(e.effect), buf);
        break;
    case EffectType::TraitModification:
        encodeTraitModification(std::get<TraitModification>(e.effect), buf);
        break;
    case EffectType::OtherEffect:
        encodeOtherEffect(std::get<OtherEffect>(e.effect), buf);
        break;
    default:
        break;
    }
}
