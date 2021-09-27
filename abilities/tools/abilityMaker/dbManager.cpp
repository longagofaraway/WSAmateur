#include "dbManager.h"

#include <stdexcept>

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>\

namespace {
void addAbilityToArray(QByteArray &data, const asn::Ability &ability) {
    auto bytes = encodeAbility(ability);
    if (bytes.size() >= (1 << 16))
        throw std::runtime_error("ability is too big");

    if (data.isEmpty()) {
        data.push_back(1);
    } else {
        data[0] = data[0] + 1;
    }

    uint16_t size = static_cast<uint16_t>(bytes.size());
    data.push_back(size & 0xFF);
    data.push_back((size >> 8) & 0xFF);
    for (size_t i = 0; i < size; ++i)
        data.push_back(bytes[i]);
}

void popAbilityFromArray(QByteArray &data) {
    if (data.isEmpty())
        return;

    data[0] = data[0] - 1;
    int n = data[0];
    if (n == 0) {
        data.clear();
        return;
    }

    int index = 1;
    while (n--) {
        uint16_t size = 0;
        size = data[index] + (data[index + 1] << 8);
        index += size + 2;
    }

    data.truncate(index);
}

void setAbilitiesToDb(QString code, QByteArray data) {
    QSqlQuery insert;
    insert.prepare("UPDATE cards SET abilities = (:abilities) WHERE code like (:code) || '%'");
    insert.bindValue(":code", code);
    insert.bindValue(":abilities", data);
    if (!insert.exec())
        throw std::runtime_error(insert.lastError().text().toStdString());
}

QByteArray getAbilitiesForCard(QString code) {
    QSqlQuery query;
    query.prepare("SELECT abilities FROM cards WHERE code = (:code)");
    query.bindValue(":code", code);
    if (!query.exec())
        throw std::runtime_error(query.lastError().text().toStdString());

    if (!query.next())
        throw std::runtime_error("Couldn't find such card in the database");

    auto var = query.value(0);
    if (var.isNull())
        return QByteArray{};
    return var.toByteArray();
}
}

DbManager::DbManager(QString path) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open()) {
        auto msg = "failed to open database at " + path;
        throw std::runtime_error(msg.toStdString());
    }
}

DbManager::~DbManager() {
    db.close();
}

void DbManager::addAbility(QString code, const asn::Ability ability) {
    auto data = getAbilitiesForCard(code);
    addAbilityToArray(data, ability);
    setAbilitiesToDb(code, data);
}

void DbManager::popAbility(QString code) {
    auto data = getAbilitiesForCard(code);
    popAbilityFromArray(data);
    setAbilitiesToDb(code, data);
}
