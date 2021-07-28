#include "jsonParser.h"

#include <stdexcept>

#include <QJsonObject>

using namespace asn;

template<typename T>
T parseConditionAndOr(const QJsonObject &json) {
    if (!json.contains("cond") || !json["cond"].isArray())
        throw std::runtime_error("no cond in ConditionAnd");
    T c;
    c.cond = parseArray(json["cond"].toArray(), parseCondition);
    return c;
}

template<typename T>
T parseNumberCard(const QJsonObject &json) {
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number");
    if (!json.contains("card") || !json["card"].isObject())
        throw std::runtime_error("no card");

    T t;
    t.number = parseNumber(json["number"].toObject());
    t.card = parseCard(json["card"].toObject());
    return t;
}


ConditionHaveCard parseConditionHaveCard(const QJsonObject &json) {
    if (json.contains("invert") && !json["invert"].isBool())
        throw std::runtime_error("no invert in ConditionHaveCard");
    if (!json.contains("who") || !json["who"].isDouble())
        throw std::runtime_error("no who in ConditionHaveCard");
    if (!json.contains("howMany") || !json["howMany"].isObject())
        throw std::runtime_error("no howMany in ConditionHaveCard");
    if (!json.contains("whichCards") || !json["whichCards"].isObject())
        throw std::runtime_error("no whichCards in ConditionHaveCard");
    if (!json.contains("where") || !json["where"].isObject())
        throw std::runtime_error("no where in ConditionHaveCard");
    if (json.contains("excludingThis") && !json["excludingThis"].isBool())
        throw std::runtime_error("no excludingThis in ConditionHaveCard");

    ConditionHaveCard c;
    if (json.contains("invert"))
        c.invert = json["invert"].toBool();
    else
        c.invert = false;
    c.who = static_cast<Player>(json["who"].toInt());
    c.howMany = parseNumber(json["howMany"].toObject());
    c.whichCards = parseCard(json["whichCards"].toObject());
    c.where = parsePlace(json["where"].toObject());
    if (json.contains("excludingThis"))
        c.excludingThis = json["excludingThis"].toBool();
    else
        c.excludingThis = false;

    return c;
}

ConditionIsCard parseConditionIsCard(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in ConditionIsCard");
    if (!json.contains("neededCard") || !json["neededCard"].isArray())
        throw std::runtime_error("no neededCard in ConditionIsCard");

    ConditionIsCard c;
    c.target = parseTarget(json["target"].toObject());
    c.neededCard = parseArray(json["neededCard"].toArray(), parseCard);

    return c;
}

ConditionCardsLocation parseConditionCardsLocation(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target in ConditionCardsLocation");
    if (!json.contains("place") || !json["place"].isObject())
        throw std::runtime_error("no place in ConditionCardsLocation");

    ConditionCardsLocation c;
    c.target = parseTarget(json["target"].toObject());
    c.place = parsePlace(json["place"].toObject());

    return c;
}

ConditionSumOfLevels parseConditionSumOfLevels(const QJsonObject &json) {
    if (!json.contains("moreThan") || !json["moreThan"].isDouble())
        throw std::runtime_error("no moreThan in ConditionSumOfLevels");

    ConditionSumOfLevels c;
    c.moreThan = json["moreThan"].toInt();

    return c;
}

ConditionDuringTurn parseConditionDuringTurn(const QJsonObject &json) {
    if (!json.contains("player") || !json["player"].isDouble())
        throw std::runtime_error("no player in ConditionDuringTurn");

    ConditionDuringTurn c;
    c.player = static_cast<Player>(json["player"].toInt());

    return c;
}

Condition parseCondition(const QJsonObject &json) {
    if (!json.contains("type") || !json["type"].isDouble())
        throw std::runtime_error("no condition type");

    Condition c;
    c.type = static_cast<ConditionType>(json["type"].toInt());
    switch (c.type) {
    case ConditionType::IsCard:
        c.cond = parseConditionIsCard(json["cond"].toObject());
        break;
    case ConditionType::HaveCards:
        c.cond = parseConditionHaveCard(json["cond"].toObject());
        break;
    case ConditionType::And:
        c.cond = parseConditionAndOr<ConditionAnd>(json["cond"].toObject());
        break;
    case ConditionType::Or:
        c.cond = parseConditionAndOr<ConditionOr>(json["cond"].toObject());
        break;
    case ConditionType::SumOfLevels:
        c.cond = parseConditionSumOfLevels(json["cond"].toObject());
        break;
    case ConditionType::CardsLocation:
        c.cond = parseConditionCardsLocation(json["cond"].toObject());
        break;
    case ConditionType::DuringTurn:
        c.cond = parseConditionDuringTurn(json["cond"].toObject());
        break;
    case ConditionType::CheckOpenedCards:
        c.cond = parseNumberCard<ConditionCheckOpenedCards>(json["cond"].toObject());
        break;
    case ConditionType::RevealedCard:
        c.cond = parseNumberCard<ConditionRevealCard>(json["cond"].toObject());
        break;
    case ConditionType::PlayersLevel:
        c.cond = parseNumberType<ConditionPlayersLevel>(json["cond"].toObject());
        break;
    case ConditionType::InBattleWithThis:
    case ConditionType::NoCondition:
        break;
    default:
        throw std::runtime_error("wrong condition type");
    }

    return c;
}
