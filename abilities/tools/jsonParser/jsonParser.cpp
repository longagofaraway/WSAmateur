#include "jsonParser.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

using namespace asn;

Ability gA;
std::vector<uint8_t> gBuf;

Target parseTarget(const QJsonObject &json);

ForEachMultiplier parseForEachMultiplier(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in ForEachMultiplier");
    if (!json.contains("zone") || !json["zone"].isDouble())
        throw std::runtime_error("no zone in ForEachMultiplier");

    ForEachMultiplier m;
    m.target = std::make_shared<Target>(parseTarget(json["target"].toObject()));
    m.zone = static_cast<Zone>(json["zone"].toInt());

    return m;
}

Multiplier parseMultiplier(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no type in Multiplier");
    if (json.contains("specifier") && !json["specifier"].isObject())
        throw std::runtime_error("no specifier in Multiplier");

    Multiplier m;
    m.type = static_cast<MultiplierType>(json["type"].toInt());
    if (m.type == MultiplierType::ForEach)
        m.specifier = parseForEachMultiplier(json["specifier"].toObject());

    return m;
}

Place parsePlace(const QJsonObject &json) {
    if (!json.contains("pos") || !json["pos"].isDouble())
        throw std::runtime_error("no pos in place");
    if (!json.contains("zone") || !json["zone"].isDouble())
        throw std::runtime_error("no zone in place");
    if (json.contains("owner") && !json["owner"].isDouble())
        throw std::runtime_error("no owner in place");

    Place p;
    p.pos = static_cast<Position>(json["pos"].toInt());
    p.zone = static_cast<Zone>(json["zone"].toInt());
    if (json.contains("owner"))
        p.owner = static_cast<Player>(json["owner"].toInt());
    else
        p.owner = Player::Player;

    return p;
}

Number parseNumber(const QJsonObject &json) {
    if (!json.contains("mod") || !json["mod"].isDouble())
        throw std::runtime_error("no NumModifier");
    if (!json.contains("value") || !json["value"].isDouble())
        throw std::runtime_error("no value in number");
    if (json.contains("multiplier") && !json["multiplier"].isObject())
        throw std::runtime_error("wrong multiplier in number");

    Number n;
    n.mod = static_cast<NumModifier>(json["mod"].toInt());
    n.value = json["value"].toInt();
    if (n.mod == NumModifier::Multiplier)
        n.multiplier = parseMultiplier(json["multiplier"].toObject());

    return n;
}

CardSpecifier parseCardSpecifier(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no type in CardSpecifier");

    CardSpecifier c;
    c.type = static_cast<CardSpecifierType>(json["type"].toInt());
    switch(c.type) {
    case CardSpecifierType::CardType:
        c.specifier = static_cast<CardType>(json["specifier"].toInt());
        break;
    case CardSpecifierType::Owner:
        c.specifier = static_cast<Player>(json["specifier"].toInt());
        break;
    case CardSpecifierType::Trait:
        c.specifier = parseStringType<Trait>(json["specifier"].toObject());
        break;
    case CardSpecifierType::ExactName:
        c.specifier = parseStringType<ExactName>(json["specifier"].toObject());
        break;
    case CardSpecifierType::NameContains:
        c.specifier = parseStringType<NameContains>(json["specifier"].toObject());
        break;
    case CardSpecifierType::Level:
        c.specifier = parseNumberType<Level>(json["specifier"].toObject());
        break;
    case CardSpecifierType::Color:
        c.specifier = static_cast<Color>(json["specifier"].toInt());
        break;
    case CardSpecifierType::Cost:
        c.specifier = parseNumberType<CostSpecifier>(json["specifier"].toObject());
        break;
    case CardSpecifierType::TriggerIcon:
        c.specifier = static_cast<TriggerIcon>(json["specifier"].toInt());
        break;
    case CardSpecifierType::HasMarker:
    case CardSpecifierType::LevelHigherThanOpp:
         break;
    default:
        throw std::runtime_error("wrong CardSpecifierType");
    }

    return c;
}

Card parseCard(const QJsonObject &json) {
    if (!json.contains("cardSpecifiers") || !json["cardSpecifiers"].isArray())
        throw std::runtime_error("no cardSpecifiers");

    Card c;
    c.cardSpecifiers = parseArray(json["cardSpecifiers"].toArray(), parseCardSpecifier);
    return c;
}

TargetSpecificCards parseTargetSpecificCards(const QJsonObject &json) {
    if (!json.contains("mode") || !json["mode"].isDouble())
        throw std::runtime_error("no mode in TargetSpecificCards");
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number in TargetSpecificCards");
    if (!json.contains("cards") || !json["cards"].isObject())
        throw std::runtime_error("no cards in TargetSpecificCards");

    TargetSpecificCards t;
    t.mode = static_cast<TargetMode>(json["mode"].toInt());
    t.number = parseNumber(json["number"].toObject());
    t.cards = parseCard(json["cards"].toObject());

    return t;
}

Target parseTarget(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no target type");
    if (json.contains("targetSpecification") && !json["targetSpecification"].isObject())
        throw std::runtime_error("wrong targetSpecification");

    Target t;
    t.type = static_cast<TargetType>(json["type"].toInt());
    if (t.type == TargetType::SpecificCards)
        t.targetSpecification = parseTargetSpecificCards(json["targetSpecification"].toObject());

    return t;
}

ZoneChangeTrigger parseZoneChangeTrigger(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isArray())
        throw std::runtime_error("no target in ZoneChangeTrigger");
    if (!json.contains("from") || !json["from"].isDouble())
        throw std::runtime_error("no from in ZoneChangeTrigger");
    if (!json.contains("to") || !json["to"].isDouble())
        throw std::runtime_error("no to in ZoneChangeTrigger");

    ZoneChangeTrigger t;
    t.target = parseArray(json["target"].toArray(), parseTarget);
    t.from = static_cast<Zone>(json["from"].toInt());
    t.to = static_cast<Zone>(json["to"].toInt());

    return t;
}

PhaseTrigger parsePhaseTrigger(const QJsonObject &json) {
    if (!json.contains("state") || !json["state"].isDouble())
        throw std::runtime_error("no state in PhaseTrigger");
    if (!json.contains("phase") || !json["phase"].isDouble())
        throw std::runtime_error("no phase in PhaseTrigger");
    if (!json.contains("owner") || !json["owner"].isDouble())
        throw std::runtime_error("no owner in PhaseTrigger");

    PhaseTrigger p;
    p.state = static_cast<PhaseState>(json["state"].toInt());
    p.phase = static_cast<Phase>(json["phase"].toInt());
    p.player = static_cast<Player>(json["owner"].toInt());

    return p;
}

OtherTrigger parseOtherTrigger(const QJsonObject &json) {
    if (!json.contains("cardCode") || !json["cardCode"].isString())
        throw std::runtime_error("no cardCode");

    OtherTrigger t;
    t.cardCode = json["cardCode"].toString().toStdString();
    return t;
}

Trigger parseTrigger(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no trigger type");
    if (json.contains("trigger") && !json["trigger"].isObject())
        throw std::runtime_error("wrong trigger");

    Trigger t;
    t.type = static_cast<TriggerType>(json["type"].toInt());
    switch (t.type) {
    case TriggerType::OnZoneChange:
        t.trigger = parseZoneChangeTrigger(json["trigger"].toObject());
        break;
    case TriggerType::OnPhaseEvent:
        t.trigger = parsePhaseTrigger(json["trigger"].toObject());
        break;
    case TriggerType::OnBattleOpponentReversed:
        t.trigger = parseCardType<BattleOpponentReversedTrigger>(json["trigger"].toObject());
        break;
    case TriggerType::OnTriggerReveal:
        t.trigger = parseCardType<TriggerRevealTrigger>(json["trigger"].toObject());
        break;
    case TriggerType::OnPlay:
        t.trigger = parseTargetType<OnPlayTrigger>(json["trigger"].toObject());
        break;
    case TriggerType::OnAttack:
        t.trigger = parseTargetType<OnAttackTrigger>(json["trigger"].toObject());
        break;
    case TriggerType::OnBackupOfThis:
    case TriggerType::OnEndOfThisCardsAttack:
    case TriggerType::OnOppCharPlacedByStandbyTriggerReveal:
    case TriggerType::OnEndOfThisTurn:
    case TriggerType::OnReversed:
         break;
    case TriggerType::OtherTrigger:
        t.trigger = parseOtherTrigger(json["trigger"].toObject());
        break;
    default:
        throw std::runtime_error("wrong trigger type");
    }

    return t;
}

StockCost parseStockCost(const QJsonObject &json) {
    if (!json.contains("value") || !json["value"].isDouble())
        throw std::runtime_error("no value in StockCost");

    StockCost e;
    e.value = json["value"].toInt();
    return e;
}

CostItem parseCostItem(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no cost type");
    if (!json.contains("costItem") || !json["costItem"].isObject())
        throw std::runtime_error("no costItem");

    CostItem i;
    i.type = static_cast<CostType>(json["type"].toInt());
    switch (i.type) {
    case CostType::Stock:
        i.costItem = parseStockCost(json["costItem"].toObject());
        break;
    case CostType::Effects:
        i.costItem = parseEffect(json["costItem"].toObject());
        break;
    default:
        throw std::runtime_error("wrong CostType");
    }

    return i;
}

Cost parseCost(const QJsonObject &json) {
    if (!json.contains("items") || !json["items"].isArray())
        throw std::runtime_error("no cost items");

    Cost cost;
    cost.items = parseArray(json["items"].toArray(), parseCostItem);
    return cost;
}

std::vector<Keyword> parseKeywords(const QJsonArray &json) {
    std::vector<Keyword> vec;
    for (const auto &elem: json)
        vec.push_back(static_cast<Keyword>(elem.toInt()));
    return vec;
}

AutoAbility parseAutoAbility(const QJsonObject &json) {
    if (json.contains("activatesUpTo") && !json["activatesUpTo"].isDouble())
        throw std::runtime_error("no activatesUpTo");
    if (!json.contains("trigger") || !json["trigger"].isObject())
        throw std::runtime_error("no trigger");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");
    if (json.contains("cost") && !json["cost"].isObject())
        throw std::runtime_error("wrong cost");
    if (json.contains("keyword") && !json["keyword"].isArray())
        throw std::runtime_error("wrong keyword");

    AutoAbility a;
    if (json.contains("activatesUpTo"))
        a.activationTimes = json["activatesUpTo"].toInt();
    else
        a.activationTimes = 0;
    a.trigger = parseTrigger(json["trigger"].toObject());
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    if (json.contains("cost"))
        a.cost = parseCost(json["cost"].toObject());
    if (json.contains("keyword"))
        a.keywords = parseKeywords(json["keyword"].toArray());

    return a;
}

ContAbility parseContAbility(const QJsonObject &json) {
    if (json.contains("keyword") && !json["keyword"].isArray())
        throw std::runtime_error("wrong keyword");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");

    ContAbility a;
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    if (json.contains("keyword"))
        a.keywords = parseKeywords(json["keyword"].toArray());

    return a;
}

ActAbility parseActAbility(const QJsonObject &json) {
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");
    if (!json.contains("cost") || !json["cost"].isObject())
        throw std::runtime_error("no cost");
    if (json.contains("keyword") && !json["keyword"].isArray())
        throw std::runtime_error("wrong keyword");

    ActAbility a;
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    a.cost = parseCost(json["cost"].toObject());
    if (json.contains("keyword"))
        a.keywords = parseKeywords(json["keyword"].toArray());

    return a;
}

EventAbility parseEventAbility(const QJsonObject &json) {
    if (json.contains("keyword") && !json["keyword"].isArray())
        throw std::runtime_error("wrong keyword");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");

    EventAbility a;
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    if (json.contains("keyword"))
        a.keywords = parseKeywords(json["keyword"].toArray());

    return a;
}

Ability parseAbility(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no ability type");
    if (!json.contains("ability") || !json["ability"].isObject())
        throw std::runtime_error("no ability");

    Ability ability;
    ability.type = static_cast<AbilityType>(json["type"].toInt());
    switch (ability.type) {
    case AbilityType::Auto:
        ability.ability = parseAutoAbility(json["ability"].toObject());
        break;
    case AbilityType::Cont:
        ability.ability = parseContAbility(json["ability"].toObject());
        break;
    case AbilityType::Act:
        ability.ability = parseActAbility(json["ability"].toObject());
        break;
    case AbilityType::Event:
        ability.ability = parseEventAbility(json["ability"].toObject());
        break;
    default:
        throw std::runtime_error("wrong ability type");
    }

    return ability;
}

QString JsonParser::createAbility(QString json) {
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (doc.isNull())
        return error.errorString();

    try {
        gA = parseAbility(doc.object());
    } catch (const std::exception &e) {
        return QString(e.what());
    }

    qDebug() << printAbility(gA).c_str();

    return QString(printAbility(gA).c_str());
}

QString JsonParser::printEncodedAbility() {
    gBuf = encodeAbility(gA);
    std::stringstream ss;
    for (size_t i = 0; i < gBuf.size(); ++i)
        ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << int(gBuf[i]) << ((i != gBuf.size() - 1) ? ", " : "");
    auto str = ss.str();
    return QString(str.c_str());
}

QString JsonParser::printDecodedAbility() {
    Ability a = decodeAbility(gBuf);
    return QString(printAbility(a).c_str());
}

QString JsonParser::initialText() {
    QFile loadFile("H:\\Projects\\Test\\WSAmatuer\\jsonKGLS79-063.txt");
    loadFile.open(QIODevice::ReadOnly);
    QString text = QString(loadFile.readAll());
    loadFile.close();
    return text;
}
