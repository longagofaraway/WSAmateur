#include "jsonParser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

using namespace asn;

Target parseTarget(const QJsonObject &json);

ForEachMultiplier parseForEachMultiplier(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in ForEachMultiplier");
    if (!json.contains("placeType") || !json["placeType"].isDouble())
        throw std::runtime_error("no placeType in ForEachMultiplier");
    if (json.contains("place") && !json["place"].isObject())
        throw std::runtime_error("wrong place in ForEachMultiplier");

    ForEachMultiplier m;
    m.target = std::make_shared<Target>(parseTarget(json["target"].toObject()));
    m.placeType = static_cast<PlaceType>(json["placeType"].toInt());
    if (m.placeType == PlaceType::SpecificPlace)
        m.place = parsePlace(json["place"].toObject());

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

    Number n;
    n.mod = static_cast<NumModifier>(json["mod"].toInt());
    n.value = json["value"].toInt();

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
    case CardSpecifierType::Power:
        c.specifier = parseNumberType<Power>(json["specifier"].toObject());
        break;
    case CardSpecifierType::HasMarker:
    case CardSpecifierType::LevelHigherThanOpp:
    case CardSpecifierType::StandbyTarget:
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
    if (t.type == TargetType::SpecificCards ||
        (t.type == TargetType::BattleOpponent && json.contains("targetSpecification")))
        t.targetSpecification = parseTargetSpecificCards(json["targetSpecification"].toObject());
    if (t.type == TargetType::BattleOpponent && !json.contains("targetSpecification")) {
        TargetSpecificCards spec;
        spec.mode = TargetMode::Any;
        spec.number = Number{NumModifier::ExactMatch, 1};
        t.targetSpecification = spec;
    }

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

StateChangeTrigger parseStateChangeTrigger(const QJsonObject &json) {
    if (!json.contains("state") || !json["state"].isDouble())
        throw std::runtime_error("no state in StateChangeTrigger");
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in StateChangeTrigger");

    StateChangeTrigger t;
    t.state = static_cast<State>(json["state"].toInt());
    t.target = parseTarget(json["target"].toObject());

    return t;
}

PhaseTrigger parsePhaseTrigger(const QJsonObject &json) {
    if (!json.contains("state") || !json["state"].isDouble())
        throw std::runtime_error("no state in PhaseTrigger");
    if (!json.contains("phase") || !json["phase"].isDouble())
        throw std::runtime_error("no phase in PhaseTrigger");
    if (!json.contains("player") || !json["player"].isDouble())
        throw std::runtime_error("no player in PhaseTrigger");

    PhaseTrigger p;
    p.state = static_cast<PhaseState>(json["state"].toInt());
    p.phase = static_cast<Phase>(json["phase"].toInt());
    p.player = static_cast<Player>(json["player"].toInt());

    return p;
}

OnBeingAttackedTrigger parseOnBeingAttackedTrigger(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in OnBeingAttackedTrigger");
    if (!json.contains("attackType") || !json["attackType"].isDouble())
        throw std::runtime_error("no attackType in OnBeingAttackedTrigger");

    OnBeingAttackedTrigger t;
    t.target = parseTarget(json["target"].toObject());
    t.attackType = static_cast<AttackType>(json["attackType"].toInt());

    return t;
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
    case TriggerType::OnStateChange:
        t.trigger = parseStateChangeTrigger(json["trigger"].toObject());
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
    case TriggerType::OnBeingAttacked:
        t.trigger = parseOnBeingAttackedTrigger(json["trigger"].toObject());
        break;
    case TriggerType::OnBackupOfThis:
    case TriggerType::OnEndOfThisCardsAttack:
    case TriggerType::OnOppCharPlacedByStandbyTriggerReveal:
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
    if (json.contains("keywords") && !json["keywords"].isArray())
        throw std::runtime_error("wrong keywords");

    AutoAbility a;
    if (json.contains("activatesUpTo"))
        a.activationTimes = json["activatesUpTo"].toInt();
    else
        a.activationTimes = 0;
    a.trigger = parseTrigger(json["trigger"].toObject());
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    if (json.contains("cost"))
        a.cost = parseCost(json["cost"].toObject());
    if (json.contains("keywords"))
        a.keywords = parseKeywords(json["keywords"].toArray());

    return a;
}

ContAbility parseContAbility(const QJsonObject &json) {
    if (json.contains("keywords") && !json["keywords"].isArray())
        throw std::runtime_error("wrong keyword");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");

    ContAbility a;
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    if (json.contains("keywords"))
        a.keywords = parseKeywords(json["keywords"].toArray());

    return a;
}

ActAbility parseActAbility(const QJsonObject &json) {
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");
    if (!json.contains("cost") || !json["cost"].isObject())
        throw std::runtime_error("no cost");
    if (json.contains("keywords") && !json["keywords"].isArray())
        throw std::runtime_error("wrong keywords");

    ActAbility a;
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    a.cost = parseCost(json["cost"].toObject());
    if (json.contains("keywords"))
        a.keywords = parseKeywords(json["keywords"].toArray());

    return a;
}

EventAbility parseEventAbility(const QJsonObject &json) {
    if (json.contains("keywords") && !json["keywords"].isArray())
        throw std::runtime_error("wrong keywords");
    if (!json.contains("effects") || !json["effects"].isArray())
        throw std::runtime_error("no effects");

    EventAbility a;
    a.effects = parseArray(json["effects"].toArray(), parseEffect);
    if (json.contains("keywords"))
        a.keywords = parseKeywords(json["keywords"].toArray());

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
