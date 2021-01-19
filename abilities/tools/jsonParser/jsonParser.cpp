#include "jsonParser.h"

#include <stdexcept>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

Target parseTarget(const QJsonObject &json);

Multiplier parseMultiplier(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no type in Multiplier");
    if (!json.contains("zone") || !json["zone"].isDouble())
        throw std::runtime_error("no zone in Multiplier");
    if (json.contains("forEachOf") && !json["forEachOf"].isObject())
        throw std::runtime_error("no zone in Multiplier");

    Multiplier m;
    m.type = static_cast<MultiplierType>(json["type"].toInt());
    m.zone = static_cast<AsnZone>(json["zone"].toInt());
    if (m.type == MultiplierType::ForEach)
        m.forEach = std::make_shared<Target>(parseTarget(json["forEachOf"].toObject()));

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
    p.zone = static_cast<AsnZone>(json["zone"].toInt());
    if (json.contains("owner"))
        p.owner = static_cast<Owner>(json["owner"].toInt());
    else
        p.owner = Owner::Player;

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
        c.specifier = static_cast<Owner>(json["specifier"].toInt());
        break;
    case CardSpecifierType::Trait:
        c.specifier = Trait{ json["specifier"].toString().toStdString() };
        break;
    case CardSpecifierType::ExactName:
        c.specifier = ExactName{ json["specifier"].toString().toStdString() };
        break;
    case CardSpecifierType::NameContains:
        c.specifier = NameContains{ json["specifier"].toString().toStdString() };
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

AsnCard parseCard(const QJsonObject &json) {
    if (!json.contains("cardSpecifiers") || !json["cardSpecifiers"].isArray())
        throw std::runtime_error("no cardSpecifiers");

    AsnCard c;
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
    t.from = static_cast<AsnZone>(json["from"].toInt());
    t.to = static_cast<AsnZone>(json["to"].toInt());

    return t;
}

PhaseTrigger parsePhaseTrigger(const QJsonObject &json) {
    if (!json.contains("state") || !json["state"].isDouble())
        throw std::runtime_error("no state in PhaseTrigger");
    if (!json.contains("phase") || !json["phase"].isDouble())
        throw std::runtime_error("no phase in PhaseTrigger");
    if (!json.contains("turnPlayer") || !json["turnPlayer"].isDouble())
        throw std::runtime_error("no turnPlayer in PhaseTrigger");

    PhaseTrigger p;
    p.state = static_cast<PhaseState>(json["state"].toInt());
    p.phase = static_cast<AsnPhase>(json["phase"].toInt());
    p.player = static_cast<AsnPlayer>(json["turnPlayer"].toInt());

    return p;
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
        t.trigger = BattleOpponentReversedTrigger{ parseCard(json["trigger"].toObject()) };
        break;
    case TriggerType::OnTriggerReveal:
        t.trigger = TriggerRevealTrigger{ parseCard(json["trigger"].toObject()) };
        break;
    case TriggerType::OnPlay:
        t.trigger = OnPlayTrigger{ parseTarget(json["trigger"].toObject()) };
        break;
    case TriggerType::OnAttack:
        t.trigger = OnPlayTrigger{ parseTarget(json["trigger"].toObject()) };
        break;
    case TriggerType::OnBackupOfThis:
    case TriggerType::OnEndOfThisCardsAttack:
    case TriggerType::OnOppCharPlacedByStandbyTriggerReveal:
    case TriggerType::OnEndOfThisTurn:
         break;
    default:
        throw std::runtime_error("wrong trigger type");
    }

    return t;
}

CostItem parseCostItem(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no cost type");

    CostItem i;
    i.type = static_cast<CostType>(json["type"].toInt());
    switch (i.type) {
    case CostType::Stock:
        if (!json.contains("costItem") || !json["costItem"].isDouble())
            throw std::runtime_error("no costItem");

        i.costItem = StockCost{ json["costItem"].toInt() };
        break;
    case CostType::Effects:
        if (!json.contains("costItem") || !json["costItem"].isObject())
            throw std::runtime_error("no costItem");

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
    if (!json.contains("activationTimes") || !json["activationTimes"].isDouble())
        throw std::runtime_error("no activationTimes");
    if (!json.contains("trigger") || !json["trigger"].isObject())
        throw std::runtime_error("no trigger");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");
    if (json.contains("cost") && !json["cost"].isObject())
        throw std::runtime_error("wrong cost");
    if (json.contains("keyword") && !json["keyword"].isArray())
        throw std::runtime_error("wrong keyword");

    AutoAbility a;
    a.activationTimes = json["activationTimes"].toInt();
    a.trigger = parseTrigger(json["trigger"].toObject());
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    if (json.contains("cost"))
        a.cost = parseCost(json["cost"].toObject());
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

    Ability a;
    try {
        a = parseAbility(doc.object());
    } catch (const std::exception &e) {
        return QString(e.what());
    }

    return QString(printAbility(a).c_str());
}

QString JsonParser::initialText() {
    QFile loadFile("F:\\Projects\\Test\\WSAmatuer\\jsonKGLS79-001.txt");
    loadFile.open(QIODevice::ReadOnly);
    QString text = QString(loadFile.readAll());
    loadFile.close();
    return text;
}
