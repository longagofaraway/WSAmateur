#include "jsonParser.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>
#include <QStandardPaths>

#include "dbManager.h"

#include "gen_cpp.h"

using namespace asn;

Ability gA;
std::vector<uint8_t> gBuf;

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
    return "";
}

QString JsonParser::addToDb(QString code, QString json) {
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (doc.isNull())
        return error.errorString();

    Ability ability;
    try {
        ability = parseAbility(doc.object());
    } catch (const std::exception &e) {
        return QString(e.what());
    }

    try {
        DbManager db;
        db.addAbility(code, ability);
    } catch (const std::exception &e) {
        return QString::fromStdString(e.what());
    }
    return "Ability added";
}

QString JsonParser::popFromDb(QString code) {
    try {
        DbManager db;
        db.popAbility(code);
    } catch (const std::exception &e) {
        return QString::fromStdString(e.what());
    }
    return "Ability removed";
}

QString JsonParser::printJsonAbility(QString code, QString pos) {
    try {
        int intPos = pos.toInt();
        if (intPos == 0)
            intPos = 1;
        DbManager db;
        auto ability = db.getAbility(code, intPos);
        return QString::fromStdString(serializeAbility(ability));
    } catch (const std::exception &e) {
        return QString::fromStdString(e.what());
    }
}

QString JsonParser::saveAbility(QString code, QString pos, QString json) {
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (doc.isNull())
        return error.errorString();

    Ability ability;
    try {
        ability = parseAbility(doc.object());
    } catch (const std::exception &e) {
        return QString(e.what());
    }

    try {
        int intPos = pos.toInt();
        if (intPos == 0)
            intPos = 1;
        DbManager db;
        db.editAbility(code, intPos, ability);
    } catch (const std::exception &e) {
        return QString::fromStdString(e.what());
    }
    return "Ability saved";
}
