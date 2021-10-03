#include "cardDatabase.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {
QString getDbPath() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir path(appData);
    path.cdUp();
    path.cd("WSAmateur");
    return path.filePath("cards.db");
}

asn::CardType convertType(int type) {
    switch (type) {
    case 0:
        return asn::CardType::Char;
    case 1:
        return asn::CardType::Event;
    case 2:
        return asn::CardType::Climax;
    default:
        throw std::runtime_error("unknown type");
    }
}

char convertColor(int color) {
    switch (color) {
    case 0:
        return 'Y';
    case 1:
        return 'G';
    case 2:
        return 'R';
    case 3:
        return 'B';
    case 4:
        return 'P';
    default:
        throw std::runtime_error("unknown color");
    }
}

std::vector<asn::TriggerIcon> parseTriggers(QString triggers) {
    auto list = triggers.split(',');
    std::vector<asn::TriggerIcon> res;
    for (auto &trigger: list) {
        if (trigger == "Soul")
            res.push_back(asn::TriggerIcon::Soul);
        else if (trigger == "Shot")
            res.push_back(asn::TriggerIcon::Shot);
        else if (trigger == "Bounce")
            res.push_back(asn::TriggerIcon::Wind);
        else if (trigger == "Choice")
            res.push_back(asn::TriggerIcon::Choice);
        else if (trigger == "GoldBar")
            res.push_back(asn::TriggerIcon::Treasure);
        else if (trigger == "Bag")
            res.push_back(asn::TriggerIcon::Bag);
        else if (trigger == "Door")
            res.push_back(asn::TriggerIcon::Door);
        else if (trigger == "Standby")
            res.push_back(asn::TriggerIcon::Standby);
        else if (trigger == "Book")
            res.push_back(asn::TriggerIcon::Book);
        else if (trigger == "Gate")
            res.push_back(asn::TriggerIcon::Gate);
    }
    return res;
}

int parseNullableInt(QVariant var) {
    if (var.isNull())
        return 0;
    return var.toInt();
}

void parseAbilities(CardInfo *info, QVariant blob) {
    if (blob.isNull())
        return;

    auto bytes = blob.toByteArray();
    if (bytes.isEmpty())
        return;

    int count = bytes[0];
    int it = 1;
    for (int i = 0; i < count; ++i) {
        uint16_t size = 0;
        size = bytes[it] + (bytes[it + 1] << 8);
        it += 2;
        std::vector<uint8_t> ability(bytes.data() + it, bytes.data() + it + size);
        info->addAbility(ability);
        it += size;
    }
}

bool parseCounter(QVariant var) {
    if (var.isNull())
        return false;

    return var.toInt();
}

void parseTraits(CardInfo *info) {
    QSqlQuery query;
    query.prepare(
"SELECT "
"   trait_id "
"FROM "
"    card_trait "
"WHERE"
"    card_code = ?");
    query.addBindValue(QString::fromStdString(info->code()));
    if (!query.exec())
        throw std::runtime_error(query.lastError().text().toStdString());

    std::vector<int> ids;
    while (query.next()) {
        ids.push_back(query.value("trait_id").toInt());
    }

    std::vector<std::string> traits;
    for (auto id: ids) {
        QSqlQuery traitQuery;
        traitQuery.prepare("SELECT trait FROM traits WHERE id = ?");
        traitQuery.addBindValue(id);
        if (!traitQuery.exec())
            throw std::runtime_error(traitQuery.lastError().text().toStdString());

        if (!traitQuery.next())
            throw std::runtime_error("trait not found");

        traits.push_back(traitQuery.value("trait").toString().toStdString());
    }

    info->setTraits(traits);
}

void parseReferences(CardInfo *info, QVariant blob) {
    if (blob.isNull())
        return;

    const QString kSep = "||";
    auto refs = blob.toString();
    if (refs.isEmpty())
        return;

    auto refList = refs.split(kSep);
    std::vector<std::string> res;
    for (const auto &ref: refList) {
        res.push_back(ref.toStdString());
    }

    info->setReferences(res);
}
}

CardDatabase::CardDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(getDbPath());

    if (!db.open()) {
        auto msg = "failed to open database at " + getDbPath();
        throw std::runtime_error(msg.toStdString());
    }

    fillCache();
}

CardDatabase& CardDatabase::get() {
    static CardDatabase instance;
    return instance;
}

std::shared_ptr<CardInfo> CardDatabase::getCard(const std::string &code) {
    if (!cards.count(code))
        return {};

    return cards.at(code);
}

void CardDatabase::fillCache() {
    QSqlQuery query;
    query.prepare(
"SELECT "
"   code, name, type, color, level, cost, soul, power, "
"   triggers, abilities, counter, card_references "
"FROM cards");
    if (!query.exec())
        throw std::runtime_error(query.lastError().text().toStdString());

    while (query.next()) {
        auto cardInfo = std::make_shared<CardInfo>();
        cardInfo->setCode(query.value("code").toString().toStdString());
        cardInfo->setName(query.value("name").toString().toStdString());
        cardInfo->setType(convertType(query.value("type").toInt()));
        cardInfo->setColor(convertColor(query.value("color").toInt()));
        cardInfo->setLevel(parseNullableInt(query.value("level")));
        cardInfo->setCost(parseNullableInt(query.value("cost")));
        cardInfo->setSoul(parseNullableInt(query.value("soul")));
        cardInfo->setPower(parseNullableInt(query.value("power")));
        cardInfo->setTriggers(parseTriggers(query.value("triggers").toString()));
        parseAbilities(cardInfo.get(), query.value("abilities"));
        cardInfo->setCounter(parseCounter(query.value("counter")));
        parseTraits(cardInfo.get());
        parseReferences(cardInfo.get(), query.value("card_references"));

        cards[cardInfo->code()] = cardInfo;
    }

}
