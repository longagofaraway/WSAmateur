#include "jsonParser.h"

#include <stdexcept>

#include <QJsonObject>

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

ChooseCard parseChooseCard(const QJsonObject &json) {
    if (json.contains("targets") && !json["targets"].isArray())
        throw std::runtime_error("wrong targets in ChooseCard");
    if (json.contains("excluding") && !json["excluding"].isArray())
        throw std::runtime_error("wrong excluding in ChooseCard");
    if (!json.contains("placeType") || !json["placeType"].isDouble())
        throw std::runtime_error("no placeType in ChooseCard");
    if (json.contains("place") && !json["place"].isObject())
        throw std::runtime_error("wrong place in ChooseCard");

    ChooseCard e;
    if (json.contains("targets"))
        e.targets = parseArray(json["targets"].toArray(), parseTarget);
    if (json.contains("excluding"))
        e.excluding = parseArray(json["excluding"].toArray(), parseCard);
    e.placeType = static_cast<PlaceType>(json["placeType"].toInt());
    if (e.placeType == PlaceType::SpecificPlace)
        e.place = parsePlace(json["place"].toObject());

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
        e.executor = static_cast<AsnPlayer>(json["executor"].toInt());
    else
        e.executor = AsnPlayer::Player;
    e.target = parseTarget(json["target"].toObject());
    e.from = parsePlace(json["from"].toObject());
    e.to = parseArray(json["to"].toArray(), parsePlace);
    if (json.contains("order"))
        e.order = static_cast<Order>(json["order"].toInt());
    else
        e.order = Order::Any;

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
        throw std::runtime_error("no targets in effect");

    SearchCard e;
    e.targets = parseArray(json["targets"].toArray(), parseSearchTarget);
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
    e.effects = parseArray(json["effects"].toArray(), parseEffect);

    return e;
}

FlipOver parseFlipOver(const QJsonObject &json) {
    if (!json.contains("number") || !json["number"].isDouble())
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

    Look e;
    e.number = parseNumber(json["number"].toObject());
    e.place = parsePlace(json["place"].toObject());

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
    if (!json.contains("modifier") || !json["modifier"].isObject())
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

    CannotUseBackupOrEvent e;
    e.what = static_cast<BackupOrEvent>(json["what"].toInt());
    e.player = static_cast<AsnPlayer>(json["player"].toInt());

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
    if (!json.contains("destination") || !json["destination"].isObject())
        throw std::runtime_error("no destination in AddMarker");

    AddMarker e;
    e.target = parseTarget(json["target"].toObject());
    e.destination = parseTarget(json["destination"].toObject());

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
        e.effect = MoveWrToDeck{ static_cast<AsnPlayer>(json["effect"].toInt()) };
        break;
    case EffectType::FlipOver:
        e.effect = parseFlipOver(json["effect"].toObject());
        break;
    case EffectType::Backup:
        e.effect = Backup{ json["effect"].toInt() };
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
        e.effect = Bond{ json["effect"].toString().toStdString() };
        break;
    case EffectType::PerformReplay:
        e.effect = PerformReplay{ json["effect"].toString().toStdString() };
        break;
    case EffectType::Replay:
        e.effect = parseReplay(json["effect"].toObject());
        break;
    case EffectType::DrawCard:
        e.effect = parseNumberType<DrawCard>(json["effect"].toObject());
        break;
    case EffectType::Look:
        e.effect = parseLook(json["effect"].toObject());
        break;
    case EffectType::TriggerCheckTwice:
    case EffectType::EarlyPlay:
    case EffectType::CannotPlay:
    case EffectType::CannotFrontAttack:
    case EffectType::CannotSideAttack:
    case EffectType::OpponentCharAutoCannotDealDamage:
    case EffectType::CannotBecomeReversed:
    case EffectType::StockSwap:
    case EffectType::CannotMove:
    case EffectType::PutRestedInSameSlot:
    case EffectType::SideAttackWithoutPenalty:
    case EffectType::Standby:
    default:
        throw std::runtime_error("wrong effect type");
    }

    return e;
}
