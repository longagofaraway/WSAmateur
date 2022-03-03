#pragma once

#include <QMutex>
#include <QObject>
#include <QStringList>

class QFile;

class ServerLogger : public QObject
{
    Q_OBJECT
private:
    QFile *logFile = nullptr;
    QStringList buffer;
    QMutex bufferMutex;

public:
    explicit ServerLogger(QObject *parent = nullptr);
    ~ServerLogger();

public slots:
    void startLog();
    void logMessage(QString message);

signals:
    void sigFlushBuffer();

private slots:
    void flushBuffer();
};

