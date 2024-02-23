#pragma once

#include <stdexcept>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QQuickItem>

#include <QDebug>

#include "abilities.h"

asn::Number parseNumber(const QJsonObject &json);
asn::Multiplier parseMultiplier(const QJsonObject &json);
asn::Place parsePlace(const QJsonObject &json);
asn::Card parseCard(const QJsonObject &json);
asn::Target parseTarget(const QJsonObject &json);
asn::Condition parseCondition(const QJsonObject &json);
asn::Effect parseEffect(const QJsonObject &json);
asn::Ability parseAbility(const QJsonObject &json);
asn::AutoAbility parseAutoAbility(const QJsonObject &json);
std::string parseString(const std::string &value);

template<typename T, typename M>
std::vector<T> parseArray(const QJsonArray &json, T(*parseType)(const M &json)) {
    if (!json.count())
        return {};

    std::vector<T> vec;
    for (const auto &elem: json) {
        if constexpr (std::is_same_v<M, QJsonObject>) {
            vec.emplace_back(parseType(elem.toObject()));
        } else if constexpr (std::is_same_v<M, std::string>) {
            vec.emplace_back(parseType(elem.toString().toStdString()));
        }
    }

    return vec;
}

template<typename T>
T parseNumberType(const QJsonObject &json) {
    if (!json.contains("value") || !json["value"].isObject())
        throw std::runtime_error("no value");

    T t;
    t.value = parseNumber(json["value"].toObject());
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

asn::EventAbility parseEventAbility(const QJsonObject &json);
class JsonParser : public QQuickItem {
    Q_OBJECT
public:
    Q_INVOKABLE QString createAbility(QString json);
    Q_INVOKABLE QString printEncodedAbility();
    Q_INVOKABLE QString printDecodedAbility();
    Q_INVOKABLE QString initialText();
    Q_INVOKABLE QString addToDb(QString code, QString json);
    Q_INVOKABLE QString popFromDb(QString code);
    Q_INVOKABLE QString printJsonAbility(QString code, QString pos);
    Q_INVOKABLE QString saveAbility(QString code, QString pos, QString json);

};
