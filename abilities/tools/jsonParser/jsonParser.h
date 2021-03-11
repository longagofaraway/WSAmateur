#pragma once

#include <stdexcept>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QQuickItem>

#include <QDebug>

#include "abilities.h"

asn::Number parseNumber(const QJsonObject &json);
template<typename T>
std::vector<T> parseArray(const QJsonArray &json, T(*parseType)(const QJsonObject &json)) {
    if (!json.count())
        return {};

    std::vector<T> vec;
    for (const auto &elem: json)
        vec.emplace_back(parseType(elem.toObject()));

    return vec;
}

template<typename T>
T parseNumberType(const QJsonObject &json) {
    if (!json.contains("number") || !json["number"].isObject())
        throw std::runtime_error("no number");

    T t;
    t.value = parseNumber(json["number"].toObject());
    return t;
}

template<typename T>
T parseStringType(const QJsonObject &json) {
    if (!json.contains("value") || !json["value"].isString())
        throw std::runtime_error("no value");

    T t;
    t.value = json["value"].toString().toStdString();
    return t;
}

template<typename T>
T parseCardType(const QJsonObject &json) {
    if (!json.contains("card") || !json["card"].isObject())
        throw std::runtime_error("no card");

    T p;
    p.card = parseCard(json["card"].toObject());
    return p;
}

template<typename T>
T parseTargetType(const QJsonObject &json) {
    if (!json.contains("target") || !json["target"].isObject())
        throw std::runtime_error("no target");

    T p;
    p.target = parseTarget(json["target"].toObject());
    return p;
}

asn::Multiplier parseMultiplier(const QJsonObject &json);
asn::Place parsePlace(const QJsonObject &json);
asn::Card parseCard(const QJsonObject &json);
asn::Target parseTarget(const QJsonObject &json);
asn::Condition parseCondition(const QJsonObject &json);
asn::Effect parseEffect(const QJsonObject &json);
asn::Ability parseAbility(const QJsonObject &json);

class JsonParser : public QQuickItem {
    Q_OBJECT
public:
    Q_INVOKABLE QString createAbility(QString json);
    Q_INVOKABLE QString printEncodedAbility();
    Q_INVOKABLE QString printDecodedAbility();
    Q_INVOKABLE QString initialText();
};
