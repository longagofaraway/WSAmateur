#include "serverLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>

namespace {
QString getLogsPath() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir path(appData);
    path.cdUp();
    path.cd("WSAmateur");
    return path.filePath("server.log");
}
}

ServerLogger::ServerLogger(QObject *parent) : QObject(parent) {}

ServerLogger::~ServerLogger() {
    flushBuffer();
    thread()->quit();
}

void ServerLogger::startLog() {
    logFile = new QFile(getLogsPath(), this);
    if (!logFile->open(QIODevice::Append)) {
        delete logFile;
        logFile = nullptr;
        return;
    }

    connect(this, SIGNAL(sigFlushBuffer()), this, SLOT(flushBuffer()), Qt::QueuedConnection);
}

void ServerLogger::logMessage(QString message) {
    if (!logFile)
        return;

    bufferMutex.lock();
    buffer.append(QDateTime::currentDateTime().toString() + " " + message);
    bufferMutex.unlock();
    emit sigFlushBuffer();
}

void ServerLogger::flushBuffer() {
    QTextStream stream(logFile);
    stream.setCodec("UTF-8");
    while (true) {
        QMutexLocker locker(&bufferMutex);
        if (buffer.isEmpty())
            return;

        QString message = buffer.takeFirst();
        locker.unlock();

        stream << message << "\n";
        stream.flush();
    }
}
