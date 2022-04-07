#include "filesystemPaths.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace {
bool cd(QDir& dir, QString dirName) {
    if (!dir.cd(dirName)) {
        if (!dir.mkdir(dirName))
            return false;
        if (!dir.cd(dirName))
            return false;
    }

    return true;
}

QDir getAppDir() {
    static QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appData);
    dir.cdUp();
    cd(dir, "WSAmateur");
    return dir;
}
}

QDir getDbDir() {
    auto dir = getAppDir();
    cd(dir, "development");
    return dir;
}

void createDirectories() {
    auto dir = getAppDir();
    cd(dir, "development");
}

void checkDeveloperDb() {
    auto dir = getDbDir();
    if (!dir.exists("cards.db")) {
        auto appDir = getAppDir();
        QFile file(appDir.filePath("cards.db"));
        if (!file.exists())
            throw std::runtime_error("Run the game once to download database");
        if (!file.copy(dir.filePath("cards.db")))
            throw std::runtime_error("Couldn't copy db to developer folder");
    }
}

