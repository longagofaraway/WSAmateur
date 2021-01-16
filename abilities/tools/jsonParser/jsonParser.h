#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QQuickItem>

#include <QDebug>

#include "abilities.h"

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

Multiplier parseMultiplier(const QJsonObject &json);
Place parsePlace(const QJsonObject &json);
Number parseNumber(const QJsonObject &json);
AsnCard parseCard(const QJsonObject &json);
Target parseTarget(const QJsonObject &json);
Condition parseCondition(const QJsonObject &json);
Effect parseEffect(const QJsonObject &json);
Ability parseAbility(const QJsonObject &json);

class JsonParser : public QQuickItem {
    Q_OBJECT
public:
    Q_INVOKABLE QString createAbility(QString json);
    Q_INVOKABLE QString initialText();
};
