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

    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir path(appData);
    path.cdUp();
    path.cd("WSAmateur");

    try {
        DbManager db(path.filePath("cards.db"));
        db.addAbility(code, ability);
    } catch (const std::exception &e) {
        return QString::fromStdString(e.what());
    }
    return "Ability added";
}

QString JsonParser::popFromDb(QString code) {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir path(appData);
    path.cdUp();
    path.cd("WSAmateur");

    try {
        DbManager db(path.filePath("cards.db"));
        db.popAbility(code);
    } catch (const std::exception &e) {
        return QString::fromStdString(e.what());
    }
    return "Ability removed";
}
