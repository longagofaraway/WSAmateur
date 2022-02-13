#pragma once

#include <QObject>

class QNetworkReply;
class QNetworkAccessManager;

class Updater : public QObject {
    Q_OBJECT
private:
    QNetworkAccessManager *networkManager = nullptr;
    QNetworkReply *response = nullptr;
    QString neededVersion;
    QString downloadUrl;

public:
    Updater(QObject *parent, const std::string &neededVersion_);

signals:
    void error(QString msg);
    void progressMade(qint64 bytesRead, qint64 totalBytes);

public slots:
    void startUpdate();

private slots:
    void releaseListReady();
    void installerDownloaded();
    void downloadProgress(qint64 bytesRead, qint64 totalBytes);

private:
    bool parseRelease(QVariantMap release);
    void downloadInstaller(QUrl url);
};
