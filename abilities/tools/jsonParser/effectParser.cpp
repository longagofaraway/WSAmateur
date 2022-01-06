#include "jsonParser.h"

#include <stdexcept>

#include <QJsonObject>

using namespace asn;

AttributeGain parseAttributeGain(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in AttributeGain");
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no type in AttributeGain");
    if (!json.contains("gainType") || !json["gainType"].isDouble())
        throw std::runtime_error("no gainType in AttributeGain");
    if (!json.contains("value") || !json["value"].isDouble())
        throw std::runtime_error("no value in AttributeGain");
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in AttributeGain");
    if (json.contains("modifier") && !json["modifier"].isObject())
        throw std::runtime_error("wrong modifier in AttributeGain");

    AttributeGain e;
    e.target = parseTarget(json["target"].toObject());
    e.type = static_cast<AttributeType>(json["type"].toInt());
    e.gainType = static_cast<ValueType>(json["gainType"].toInt());
    e.value = json["value"].toInt();
    e.duration = json["duration"].toInt();
    if (e.gainType == ValueType::Multiplier)
        e.modifier = parseMultiplier(json["modifier"].toObject());

    return e;
}

TargetAndPlace parseTargetAndPlace(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in TargetAndPlace");
    if (!json.contains("placeType") || !json["placeType"].isDouble())
        throw std::runtime_error("no placeType in TargetAndPlace");
    if (json.contains("place") && !json["place"].isObject())
        throw std::runtime_error("wrong place in TargetAndPlace");

    TargetAndPlace t;
    t.target = parseTarget(json["target"].toObject());
    t.placeType = static_cast<PlaceType>(json["placeType"].toInt());
    if (t.placeType == PlaceType::SpecificPlace)
        t.place = parsePlace(json["place"].toObject());
    return t;
}

ChooseCard parseChooseCard(const QJsonObject &json) {
    if (json.contains("executor") && !json["executor"].isDouble())
        throw std::runtime_error("wrong executor in ChooseCard");
    if (!json.contains("targets") || !json["targets"].isArray())
        throw std::runtime_error("no targets in ChooseCard");
    if (json.contains("excluding") && !json["excluding"].isArray())
        throw std::runtime_error("wrong excluding in ChooseCard");

    ChooseCard e;
    if (json.contains("executor"))
        e.executor = static_cast<Player>(json["executor"].toInt());
    else
        e.executor = Player::Player;
    e.targets = parseArray(json["targets"].toArray(), parseTargetAndPlace);
    if (json.contains("excluding"))
        e.excluding = parseArray(json["excluding"].toArray(), parseCard);

    return e;
}

RevealCard parseRevealCard(const QJsonObject &json) {
    if (!json.contains("revealType") || !json["revealType"].isDouble())
        throw std::runtime_error("no revealType in RevealCard");
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number in RevealCard");
    if (json.contains("card") && !json["card"].isObject())
        throw std::runtime_error("wrong card in RevealCard");

    RevealCard e;
    e.type = static_cast<RevealType>(json["revealType"].toInt());
    e.number = parseNumber(json["number"].toObject());
    if (e.type == RevealType::FromHand)
        e.card = parseCard(json["card"].toObject());

    return e;
}

MoveCard parseMoveCard(const QJsonObject &json) {
    if (json.contains("executor") && !json["executor"].isDouble())
        throw std::runtime_error("wrong executor in MoveCard");
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in MoveCard");
    if (!json.contains("from") || !json["from"].isObject())
        throw std::runtime_error("no from in MoveCard");
    if (!json.contains("to") || !json["to"].isArray())
        throw std::runtime_error("no revealType in MoveCard");
    if (json.contains("order") && !json["order"].isDouble())
        throw std::runtime_error("wrong order in MoveCard");

    MoveCard e;
    if (json.contains("executor"))
        e.executor = static_cast<Player>(json["executor"].toInt());
    else
        e.executor = Player::Player;
    e.target = parseTarget(json["target"].toObject());
    e.from = parsePlace(json["from"].toObject());
    e.to = parseArray(json["to"].toArray(), parsePlace);
    if (json.contains("order"))
        e.order = static_cast<Order>(json["order"].toInt());
    else
        e.order = Order::NotSpecified;

    return e;
}

SearchTarget parseSearchTarget(const QJsonObject &json) {
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number in SearchTarget");
    if (!json.contains("cards") || !json["cards"].isArray())
        throw std::runtime_error("no cards in SearchTarget");

    SearchTarget s;
    s.number = parseNumber(json["number"].toObject());
    s.cards = parseArray(json["cards"].toArray(), parseCard);

    return s;
}

SearchCard parseSearchCard(const QJsonObject &json) {
    if (!json.contains("targets") || !json["targets"].isArray())
        throw std::runtime_error("no targets in SearchCard");
    if (json.contains("place") && !json["place"].isObject())
        throw std::runtime_error("wrong place in SearchCard");

    SearchCard e;
    e.targets = parseArray(json["targets"].toArray(), parseSearchTarget);
    if (json.contains("place"))
        e.place = parsePlace(json["place"].toObject());
    else {
        Place p { Position::NotSpecified, Zone::Deck, Player::Player };
        e.place = p;
    }
    return e;
}

PayCost parsePayCost(const QJsonObject &json) {
    if (!json.contains("ifYouDo") || !json["ifYouDo"].isArray())
        throw std::runtime_error("no ifYouDo in PayCost");
    if (!json.contains("ifYouDont") || !json["ifYouDont"].isArray())
        throw std::runtime_error("no ifYouDont in PayCost");

    PayCost e;
    e.ifYouDo = parseArray(json["ifYouDo"].toArray(), parseEffect);
    e.ifYouDont = parseArray(json["ifYouDont"].toArray(), parseEffect);

    return e;
}

AbilityGain parseAbilityGain(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in AbilityGain");
    if (!json.contains("number") || !json["number"].isDouble())
        throw std::runtime_error("no number in AbilityGain");
    if (!json.contains("abilities") || !json["abilities"].isArray())
        throw std::runtime_error("no abilities in AbilityGain");
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in AbilityGain");

    AbilityGain e;
    e.target = parseTarget(json["target"].toObject());
    e.number = json["number"].toInt();
    e.abilities = parseArray(json["abilities"].toArray(), parseAbility);
    e.duration = json["duration"].toInt();

    return e;
}

PerformEffect parsePerformEffect(const QJsonObject &json) {
    if (!json.contains("numberOfEffects") || !json["numberOfEffects"].isDouble())
        throw std::runtime_error("no numberOfEffects in PerformEffect");
    if (!json.contains("numberOfTimes") || !json["numberOfTimes"].isDouble())
        throw std::runtime_error("no numberOfTimes in PerformEffect");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects in PerformEffect");

    PerformEffect e;
    e.numberOfEffects = json["numberOfEffects"].toInt();
    e.numberOfTimes = json["numberOfTimes"].toInt();
    e.effects = parseArray(json["effects"].toArray(), parseEventAbility);

    return e;
}

FlipOver parseFlipOver(const QJsonObject &json) {
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number in FlipOver");
    if (!json.contains("forEach") || !json["forEach"].isObject())
        throw std::runtime_error("no forEach in FlipOver");
    if (!json.contains("effect") || !json["effect"].isArray())
        throw std::runtime_error("no effect in FlipOver");

    FlipOver e;
    e.number = parseNumber(json["number"].toObject());
    e.forEach = parseCard(json["forEach"].toObject());
    e.effect = parseArray(json["effect"].toArray(), parseEffect);

    return e;
}

Look parseLook(const QJsonObject &json) {
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number in Look");
    if (!json.contains("place") || !json["place"].isObject())
        throw std::runtime_error("no place in Look");
    if (json.contains("valueType") && !json["valueType"].isDouble())
        throw std::runtime_error("no valueType in Look");
    if (json.contains("multiplier") && !json["multiplier"].isObject())
        throw std::runtime_error("no multiplier in Look");

    Look e;
    e.number = parseNumber(json["number"].toObject());
    e.place = parsePlace(json["place"].toObject());
    e.valueType = static_cast<ValueType>(json["valueType"].toInt());
    if (e.valueType == ValueType::Multiplier)
        e.multiplier = parseMultiplier(json["multiplier"].toObject());

    return e;
}

NonMandatory parseNonMandatory(const QJsonObject &json) {
    if (!json.contains("effect") || !json["effect"].isArray())
        throw std::runtime_error("no effect in NonMandatory");
    if (!json.contains("ifYouDo") || !json["ifYouDo"].isArray())
        throw std::runtime_error("no ifYouDo in NonMandatory");
    if (!json.contains("ifYouDont") || !json["ifYouDont"].isArray())
        throw std::runtime_error("no ifYouDont in NonMandatory");

    NonMandatory e;
    e.effect = parseArray(json["effect"].toArray(), parseEffect);
    e.ifYouDo = parseArray(json["ifYouDo"].toArray(), parseEffect);
    e.ifYouDont = parseArray(json["ifYouDont"].toArray(), parseEffect);

    return e;
}

ChangeState parseChangeState(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in ChangeState");
    if (!json.contains("state") || !json["state"].isDouble())
        throw std::runtime_error("no state in ChangeState");

    ChangeState e;
    e.target = parseTarget(json["target"].toObject());
    e.state = static_cast<State>(json["state"].toInt());

    return e;
}

DealDamage parseDealDamage(const QJsonObject &json) {
    if (!json.contains("damageType") || !json["damageType"].isDouble())
        throw std::runtime_error("no damageType in DealDamage");
    if (!json.contains("damage") || !json["damage"].isDouble())
        throw std::runtime_error("no damage in DealDamage");
    if (json.contains("modifier") && !json["modifier"].isObject())
        throw std::runtime_error("no modifier in DealDamage");

    DealDamage e;
    e.damageType = static_cast<ValueType>(json["damageType"].toInt());
    e.damage = json["damage"].toInt();
    if (e.damageType == ValueType::Multiplier)
        e.modifier = parseMultiplier(json["modifier"].toObject());

    return e;
}

CannotUseBackupOrEvent parseCannotUseBackupOrEvent(const QJsonObject &json) {
    if (!json.contains("what") || !json["what"].isDouble())
        throw std::runtime_error("no what in CannotUseBackupOrEvent");
    if (!json.contains("player") || !json["player"].isDouble())
        throw std::runtime_error("no player in CannotUseBackupOrEvent");
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in AbilityGain");

    CannotUseBackupOrEvent e;
    e.what = static_cast<BackupOrEvent>(json["what"].toInt());
    e.player = static_cast<Player>(json["player"].toInt());
    e.duration = json["duration"].toInt();

    return e;
}

SwapCards parseSwapCards(const QJsonObject &json) {
    if (!json.contains("chooseFirst") || !json["chooseFirst"].isObject())
        throw std::runtime_error("no chooseFirst in SwapCards");
    if (!json.contains("chooseSecond") || !json["chooseSecond"].isObject())
        throw std::runtime_error("no chooseSecond in SwapCards");

    SwapCards e;
    e.first = parseChooseCard(json["chooseFirst"].toObject());
    e.second = parseChooseCard(json["chooseSecond"].toObject());

    return e;
}

AddMarker parseAddMarker(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in AddMarker");
    if (!json.contains("from") || !json["from"].isObject())
        throw std::runtime_error("no from in AddMarker");
    if (!json.contains("destination") || !json["destination"].isObject())
        throw std::runtime_error("no destination in AddMarker");
    if (json.contains("orientation") && !json["orientation"].isDouble())
        throw std::runtime_error("wrong orientation in DrawCard");

    AddMarker e;
    e.target = parseTarget(json["target"].toObject());
    e.from = parsePlace(json["from"].toObject());
    e.destination = parseTarget(json["destination"].toObject());
    if (json.contains("orientation"))
        e.orientation = static_cast<FaceOrientation>(json["orientation"].toInt());
    else
        e.orientation = FaceOrientation::FaceDown;
    return e;
}

Replay parseReplay(const QJsonObject &json) {
    if (!json.contains("name") || !json["name"].isString())
        throw std::runtime_error("no name in Replay");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects in Replay");

    Replay e;
    e.name = json["name"].toString().toStdString();
    e.effects = parseArray(json["effects"].toArray(), parseEffect);

    return e;
}

DrawCard parseDrawCard(const QJsonObject &json) {
    if (json.contains("executor") && !json["executor"].isDouble())
        throw std::runtime_error("wrong executor in DrawCard");
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number in DrawCard");

    DrawCard e;
    if (json.contains("executor"))
        e.executor = static_cast<Player>(json["executor"].toInt());
    else
        e.executor = Player::Player;
    e.value = parseNumber(json["number"].toObject());

    return e;
}

Shuffle parseShuffle(const QJsonObject &json) {
    if (!json.contains("zone") || !json["zone"].isDouble())
        throw std::runtime_error("no zone in Shuffle");
    if (json.contains("owner") && !json["owner"].isDouble())
        throw std::runtime_error("wrong owner in Shuffle");

    Shuffle e;
    e.zone = static_cast<Zone>(json["zone"].toInt());
    if (json.contains("owner"))
        e.owner = static_cast<Player>(json["owner"].toInt());
    else
        e.owner = Player::Player;

    return e;
}

Backup parseBackup(const QJsonObject &json) {
    if (!json.contains("power") || !json["power"].isDouble())
        throw std::runtime_error("no power in Backup");
    if (!json.contains("level") || !json["level"].isDouble())
        throw std::runtime_error("no level in Backup");

    Backup e;
    e.power = json["power"].toInt();
    e.level = json["level"].toInt();
    return e;
}

MoveWrToDeck parseMoveWrToDeck(const QJsonObject &json) {
    if (!json.contains("executor") || !json["executor"].isDouble())
        throw std::runtime_error("no executor in MoveWrToDeck");

    MoveWrToDeck e;
    e.executor = static_cast<Player>(json["executor"].toInt());
    return e;
}

CannotAttack parseCannotAttack(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in CannotAttack");
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no type in CannotAttack");
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in CannotAttack");

    CannotAttack e;
    e.target = parseTarget(json["target"].toObject());
    e.type = static_cast<AttackType>(json["type"].toInt());
    e.duration = json["duration"].toInt();
    return e;
}

CannotBecomeReversed parseCannotBecomeReversed(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in CannotAttack");
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in CannotAttack");

    CannotBecomeReversed e;
    e.target = parseTarget(json["target"].toObject());
    e.duration = json["duration"].toInt();
    return e;
}

OtherEffect parseOtherEffect(const QJsonObject &json) {
    if (!json.contains("cardCode") || !json["cardCode"].isString())
        throw std::runtime_error("no cardCode in OtherEffect");
    if (!json.contains("effectId") || !json["effectId"].isDouble())
        throw std::runtime_error("no effectId in OtherEffect");

    OtherEffect e;
    e.cardCode = json["cardCode"].toString().toStdString();
    e.effectId = json["effectId"].toInt();
    return e;
}

OpponentAutoCannotDealDamage parseOpponentAutoCannotDealDamage(const QJsonObject &json) {
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in CannotAttack");

    OpponentAutoCannotDealDamage e;
    e.duration = json["duration"].toInt();
    return e;
}

CannotMove parseCannotMove(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in CannotMove");
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in CannotMove");

    CannotMove e;
    e.target = parseTarget(json["target"].toObject());
    e.duration = json["duration"].toInt();
    return e;
}

SideAttackWithoutPenalty parseSideAttackWithoutPenalty(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in CannotMove");
    if (!json.contains("duration") || !json["duration"].isDouble())
        throw std::runtime_error("no duration in SideAttackWithoutPenalty");

    SideAttackWithoutPenalty e;
    e.target = parseTarget(json["target"].toObject());
    e.duration = json["duration"].toInt();
    return e;
}

PutOnStageRested parsePutOnStageRested(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in PutOnStageRested");
    if (!json.contains("from") || !json["from"].isObject())
        throw std::runtime_error("no from in PutOnStageRested");
    if (!json.contains("to") || !json["to"].isDouble())
        throw std::runtime_error("no to in PutOnStageRested");

    PutOnStageRested e;
    e.target = parseTarget(json["target"].toObject());
    e.from = parsePlace(json["from"].toObject());
    e.to = static_cast<Position>(json["to"].toInt());
    return e;
}

RemoveMarker parseRemoveMarker(const QJsonObject &json) {
    if (!json.contains("targetMarker") || !json["targetMarker"].isObject())
        throw std::runtime_error("no targetMarker in RemoveMarker");
    if (!json.contains("markerBearer") || !json["markerBearer"].isObject())
        throw std::runtime_error("no markerBearer in RemoveMarker");
    if (!json.contains("place") || !json["place"].isObject())
        throw std::runtime_error("no place in RemoveMarker");

    RemoveMarker e;
    e.targetMarker = parseTarget(json["targetMarker"].toObject());
    e.markerBearer = parseTarget(json["markerBearer"].toObject());
    e.place = parsePlace(json["place"].toObject());
    return e;
}

Effect parseEffect(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no effect type");
    if (json.contains("condition") && !json["condition"].isObject())
        throw std::runtime_error("wrong condition type");

    Effect e;
    e.type = static_cast<EffectType>(json["type"].toInt());
    if (json.contains("condition"))
        e.cond = parseCondition(json["condition"].toObject());
    else
        e.cond.type = ConditionType::NoCondition;
    switch (e.type) {
    case EffectType::AttributeGain:
        e.effect = parseAttributeGain(json["effect"].toObject());
        break;
    case EffectType::ChooseCard:
        e.effect = parseChooseCard(json["effect"].toObject());
        break;
    case EffectType::RevealCard:
        e.effect = parseRevealCard(json["effect"].toObject());
        break;
    case EffectType::MoveCard:
        e.effect = parseMoveCard(json["effect"].toObject());
        break;
    case EffectType::SearchCard:
        e.effect = parseSearchCard(json["effect"].toObject());
        break;
    case EffectType::PayCost:
        e.effect = parsePayCost(json["effect"].toObject());
        break;
    case EffectType::AbilityGain:
        e.effect = parseAbilityGain(json["effect"].toObject());
        break;
    case EffectType::PerformEffect:
        e.effect = parsePerformEffect(json["effect"].toObject());
        break;
    case EffectType::MoveWrToDeck:
        e.effect = parseMoveWrToDeck(json["effect"].toObject());
        break;
    case EffectType::FlipOver:
        e.effect = parseFlipOver(json["effect"].toObject());
        break;
    case EffectType::Backup:
        e.effect = parseBackup(json["effect"].toObject());
        break;
    case EffectType::NonMandatory:
        e.effect = parseNonMandatory(json["effect"].toObject());
        break;
    case EffectType::ChangeState:
        e.effect = parseChangeState(json["effect"].toObject());
        break;
    case EffectType::DealDamage:
        e.effect = parseDealDamage(json["effect"].toObject());
        break;
    case EffectType::CannotUseBackupOrEvent:
        e.effect = parseCannotUseBackupOrEvent(json["effect"].toObject());
        break;
    case EffectType::SwapCards:
        e.effect = parseSwapCards(json["effect"].toObject());
        break;
    case EffectType::AddMarker:
        e.effect = parseAddMarker(json["effect"].toObject());
        break;
    case EffectType::Bond:
        e.effect = parseStringType<Bond>(json["effect"].toObject());
        break;
    case EffectType::PerformReplay:
        e.effect = parseStringType<PerformReplay>(json["effect"].toObject());
        break;
    case EffectType::Replay:
        e.effect = parseReplay(json["effect"].toObject());
        break;
    case EffectType::DrawCard:
        e.effect = parseDrawCard(json["effect"].toObject());
        break;
    case EffectType::Look:
        e.effect = parseLook(json["effect"].toObject());
        break;
    case EffectType::Shuffle:
        e.effect = parseShuffle(json["effect"].toObject());
        break;
    case EffectType::CannotAttack:
        e.effect = parseCannotAttack(json["effect"].toObject());
        break;
    case EffectType::CannotBecomeReversed:
        e.effect = parseCannotBecomeReversed(json["effect"].toObject());
        break;
    case EffectType::OpponentAutoCannotDealDamage:
        e.effect = parseOpponentAutoCannotDealDamage(json["effect"].toObject());
        break;
    case EffectType::CannotMove:
        e.effect = parseCannotMove(json["effect"].toObject());
        break;
    case EffectType::SideAttackWithoutPenalty:
        e.effect = parseSideAttackWithoutPenalty(json["effect"].toObject());
        break;
    case EffectType::PutOnStageRested:
        e.effect = parsePutOnStageRested(json["effect"].toObject());
        break;
    case EffectType::RemoveMarker:
        e.effect = parseRemoveMarker(json["effect"].toObject());
        break;
    case EffectType::TriggerCheckTwice:
    case EffectType::EarlyPlay:
    case EffectType::CannotPlay:
    case EffectType::CharAutoCannotDealDamage:
    case EffectType::StockSwap:
    case EffectType::Standby:
        break;
    case EffectType::OtherEffect:
        e.effect = parseOtherEffect(json["effect"].toObject());
        break;
    default:
        throw std::runtime_error("wrong effect type");
    }

    return e;
}
