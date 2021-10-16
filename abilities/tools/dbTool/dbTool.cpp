#include <iostream>

#include <QDir>
#include <QString>
#include <QVariant>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QtCore>

#include "abilities.h"
#include "dbManager.h"

#include <QDebug>

namespace {
std::string gCommand;
std::string gParam;

std::string help() {
    std::string h;
    h = "Usage: dbTool <command>\n";
    h += "  Commands:\n";
    h += "    transform - decodes all abilities and encodes them back to db\n";
    h += "    print <card_id> - print abilities text of a card\n";
    return h;
}

std::vector<asn::Ability> parseAbilities(QByteArray buf) {
    std::vector<asn::Ability> abilities;
    int count = buf[0];
    int it = 1;
    for (int i = 0; i < count; ++i) {
        uint16_t size = 0;
        size = static_cast<uint8_t>(buf[it]) + (static_cast<uint8_t>(buf[it + 1]) << 8);
        it += 2;
        std::vector<uint8_t> ability(buf.data() + it, buf.data() + it + size);
        abilities.push_back(decodeAbility(ability));
        it += size;
    }
    return abilities;
}

void updateAbilities(QString code, std::vector<asn::Ability> &abilities) {
    QByteArray data;
    for (const auto &a: abilities) {
        auto buf = encodeAbility(a);

        if (data.isEmpty()) {
            data.push_back(1);
        } else {
            data[0] = data[0] + 1;
        }

        uint16_t size = static_cast<uint16_t>(buf.size());
        data.push_back(size & 0xFF);
        data.push_back((size >> 8) & 0xFF);
        for (size_t i = 0; i < size; ++i)
            data.push_back(buf[i]);
    }

    QSqlQuery insert;
    insert.prepare("UPDATE cards SET abilities = (:abilities) WHERE code = (:code)");
    insert.bindValue(":code", code);
    insert.bindValue(":abilities", data);
    if (!insert.exec())
        throw std::runtime_error(insert.lastError().text().toStdString());
}

QString getDbPath() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir path(appData);
    path.cdUp();
    path.cd("WSAmateur");
    return path.filePath("cards.db");
}

void transform() {
    DbManager dbManager(getDbPath());

    QSqlQuery query;
    query.prepare("SELECT code, abilities FROM cards");
    if (!query.exec())
        throw std::runtime_error(query.lastError().text().toStdString());

    int recordCount = 0;
    while (query.next()) {
        recordCount++;
        auto var = query.value(1);
        if (var.isNull())
            continue;
        auto buf = var.toByteArray();
        if (buf.isEmpty())
            return;

        auto code = query.value(0).toString();

        auto abilities = parseAbilities(buf);
        updateAbilities(code, abilities);
    }
    std::cout << "processed " << recordCount << '\n';
}

void print(std::string cardId) {
    DbManager dbManager(getDbPath());

    QSqlQuery query;
    query.prepare("SELECT abilities FROM cards WHERE code = (:code)");
    query.bindValue(":code", QString::fromStdString(cardId));
    if (!query.exec())
        throw std::runtime_error(query.lastError().text().toStdString());

    if (!query.next()) {
        std::cout << "card not found\n";
        return;
    }

    auto var = query.value(0);
    if (var.isNull()) {
        std::cout << "no abilities\n";
        return;
    }

    auto buf = var.toByteArray();
    if (buf.isEmpty()) {
        std::cout << "empty abilities\n";
        return;
    }

    auto ab = parseAbilities(buf);
    for (auto &a: ab) {
        std::cout << printAbility(a) << "\n\n";
    }
}
}

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = 0) : QObject(parent) {}

public slots:
    void run()
    {
        try {
            if (gCommand == "transform")
                transform();
            else if (gCommand == "print")
                print(gParam);
        } catch (const std::exception &e) {
            std::cout << e.what() << '\n';
        }

        emit finished();
    }

signals:
    void finished();
};

#include "dbTool.moc"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << help();
        return 1;
    }

    if (!strcmp(argv[1], "transform")) {
        gCommand = "transform";
    } else if (!strcmp(argv[1], "print") && argc > 2) {
        gCommand = "print";
        gParam = argv[2];
    } else {
        std::cout << help();
        return 1;
    }

    QCoreApplication a(argc, argv);

    Task *task = new Task(&a);

    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));

    QTimer::singleShot(0, task, SLOT(run()));

    return a.exec();
}
