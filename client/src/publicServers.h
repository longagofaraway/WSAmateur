#pragma once

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class PublicServers : public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager *networkManager;
    QNetworkReply *reply = nullptr;
    QString serverAddress;
    QString serverPort;

public:
    PublicServers();

    void getServers();

signals:
    void serversReady(QString address, QString port);

private slots:
    void serversDataDownloaded();
};

