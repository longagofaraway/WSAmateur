#include "filesystemPaths.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

namespace paths {

namespace {
QDir getAppDir() {
    static QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    qDebug() << appData;
    return QDir(appData);
}
}

QString imageLinksPath() {
    auto dir = getAppDir();
    return dir.filePath("cardImageLinks.json");
}

}
