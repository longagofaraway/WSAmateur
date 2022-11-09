#include "dbManager.h"

#include <stdexcept>

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include "filesystemPaths.h"

namespace {
// db format ability
// first byte - number of abilities
// second, third bytes - length of the next ability, little endian
// n bytes of the ability
// n+1, n+2 bytes - length of the next ability...

void validateAbility(const asn::Ability &a) {
    if (a.type != asn::AbilityType::Event)
        return;

    const auto &eventAbility = std::get<asn::EventAbility>(a.ability);
    if (std::any_of(eventAbility.effects.begin(), eventAbility.effects.end(),
                    [](const asn::Effect &effect) {
            return effect.type == asn::EffectType::CannotPlay;
        }))
        throw std::runtime_error("Please use CannotPlay effect in Continuous abilities only. "
                                 "Ability is not added");
}

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
        size = static_cast<uint8_t>(data[index]) + (static_cast<uint8_t>(data[index + 1]) << 8);
        index += size + 2;
    }

    data.truncate(index);
}

void setAbilitiesToDb(QString code, QByteArray data) {
    QSqlQuery insert;
    insert.prepare("UPDATE cards SET abilities = (:abilities) WHERE code like (:code) || '%'");
    insert.bindValue(":abilities", data);
    insert.bindValue(":code", code);
    if (!insert.exec()) {
        qWarning() << insert.lastError().text();
        throw std::runtime_error(insert.lastError().text().toStdString());
    }
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

void replaceAbility(QByteArray& data, int pos, const asn::Ability ability) {
    auto bytes = encodeAbility(ability);
    QByteArray newAbility(reinterpret_cast<const char*>(bytes.data()),
                          static_cast<int>(bytes.size()));
    if (bytes.size() >= (1 << 16))
        throw std::runtime_error("ability is too big");
    if (data.isEmpty())
        throw std::runtime_error("no abilities for the card");
    int number = data[0];
    if (pos > number)
        throw std::runtime_error("wrong index");

    int cur = 0;
    int index = 1;
    while (cur < number) {
        cur++;
        uint16_t size = static_cast<uint8_t>(data[index]) +
                       (static_cast<uint8_t>(data[index + 1]) << 8);
        index += 2;
        if (cur == pos) {
            data.replace(index, size, newAbility);
            break;
        }
    }
}
} // namespace

DbManager::DbManager() {
    prepareDbFile();
    db = QSqlDatabase::addDatabase("QSQLITE");
    auto dir = getDbDir();
    auto path = dir.filePath("cards.db");
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
    validateAbility(ability);
    auto data = getAbilitiesForCard(code);
    addAbilityToArray(data, ability);
    setAbilitiesToDb(code, data);
}

void DbManager::popAbility(QString code) {
    auto data = getAbilitiesForCard(code);
    popAbilityFromArray(data);
    setAbilitiesToDb(code, data);
}

asn::Ability DbManager::getAbility(QString code, int pos) {
    auto data = getAbilitiesForCard(code);
    int abilityNumber = data[0];

    if (pos > abilityNumber)
        throw std::runtime_error("wrong index");

    int cur = 0, n = 1;
    QByteArray byteAbility;
    while (cur < pos) {
        cur++;
        uint16_t size = static_cast<uint8_t>(data[n]) +
                       (static_cast<uint8_t>(data[n + 1]) << 8);
        n += 2;
        if (cur == pos) {
            byteAbility = data.mid(n, size);
            break;
        }
        n += size;
    }

    std::vector<uint8_t> buf(byteAbility.begin(), byteAbility.end());
    return decodeAbility(buf);
}

void DbManager::editAbility(QString code, int pos, const asn::Ability ability) {
    validateAbility(ability);
    auto data = getAbilitiesForCard(code);
    replaceAbility(data, pos, ability);
    setAbilitiesToDb(code, data);
}

void DbManager::prepareDbFile() {
    createDirectories();
    checkDeveloperDb();
}
