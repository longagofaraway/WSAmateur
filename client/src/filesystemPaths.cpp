#include "filesystemPaths.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

namespace paths {

namespace {
const QString kPicsDirName = "downloadedPics";
QDir getAppDir() {
    static QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appData);
}

bool cd(QDir& dir, QString dirName) {
    if (!dir.cd(dirName)) {
        if (!dir.mkdir(dirName))
            return false;
        if (!dir.cd(dirName))
            return false;
    }

    return true;
}
}

QString imageLinksPath() {
    auto dir = getAppDir();
    return dir.filePath("cardImageLinks.json");
}

QString settingsPath() {
    auto dir = getAppDir();
    return dir.filePath("settings.ini");
}

QString cardImagePath(const std::string &code) {
    return cardImagePath(QString::fromStdString(code));
}

QString cardImagePath(QString code) {
    auto dir = getAppDir();
    if (!cd(dir, kPicsDirName))
        return "";

    auto start = code.indexOf('/') + 1;
    auto end = code.indexOf('-');
    QString set = code.mid(start, end - start);

    if (!cd(dir, set))
        return "";

    code.remove('/');
    return dir.filePath(code);
}

bool cardImageExists(const std::string &code) {
    auto dir = getAppDir();
    if (!cd(dir, kPicsDirName))
        return false;

    QString fileName = QString::fromStdString(code);

    auto start = fileName.indexOf('/') + 1;
    auto end = fileName.indexOf('-');
    QString set = fileName.mid(start, end - start);
    if (!cd(dir, set))
        return false;

    fileName.remove('/');
    auto fileInfoList = dir.entryInfoList();
    for (const auto &fileInfo: fileInfoList) {
        if (fileInfo.baseName() == fileName)
            return true;
    }
    return false;
}

QDir decksDir() {
    auto dir = getAppDir();
    cd(dir, "decks");
    return dir;
}

void setUpRootDirectory() {
    auto dir = getAppDir();
    if (!dir.exists())
        dir.mkpath(dir.absolutePath());
}

}
