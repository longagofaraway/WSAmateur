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

void encodeChooseCard(const ChooseCard &e, Buf &buf) {
    encodeArray(e.targets, buf, encodeTarget);
    encodeArray(e.excluding, buf, encodeCard);
    buf.push_back(static_cast<uint8_t>(e.placeType));
    if (e.place)
        encodePlace(*e.place, buf);
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
    encodeArray(e.effects, buf, encodeEffect);
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
}

void encodeSwapCards(const SwapCards &e, Buf &buf) {
    encodeChooseCard(e.first, buf);
    encodeChooseCard(e.second, buf);
}

void encodeAddMarker(const AddMarker &e, Buf &buf) {
    encodeTarget(e.target, buf);
    encodeTarget(e.destination, buf);
}

void encodeReplay(const Replay &e, Buf &buf) {
    encodeString(e.name, buf);
    encodeArray(e.effects, buf, encodeEffect);
}

void encodeLook(const Look &e, Buf &buf) {
    encodeNumber(e.number, buf);
    encodePlace(e.place, buf);
}

void encodeDrawCard(const DrawCard &e, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(e.executor));
    encodeNumber(e.value, buf);
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
        toBufLE(zzenc_32(std::get<Backup>(e.effect).power), buf);
        break;
    case EffectType::NonMandatory:
        encodeNonMandatory(std::get<NonMandatory>(e.effect), buf);
        break;
    case EffectType::ChangeState:
        encodeChangeState(std::get<ChangeState>(e.effect), buf);
        break;
    case EffectType::DealDamage:
        encodeDealDamage(std::get<DealDamage>(e.effect), buf);
        break;
    case EffectType::CannotUseBackupOrEvent:
        encodeCannotUseBackupOrEvent(std::get<CannotUseBackupOrEvent>(e.effect), buf);
        break;
    case EffectType::SwapCards:
        encodeSwapCards(std::get<SwapCards>(e.effect), buf);
        break;
    case EffectType::AddMarker:
        encodeAddMarker(std::get<AddMarker>(e.effect), buf);
        break;
    case EffectType::Bond:
        encodeString(std::get<Bond>(e.effect).name, buf);
        break;
    case EffectType::PerformReplay:
        encodeString(std::get<PerformReplay>(e.effect).name, buf);
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
    default:
        break;
    }
}
