#include "publicServers.h"

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define PUBLIC_SERVERS_JSON "https://longagofaraway.github.io/public-servers.json"

PublicServers::PublicServers() :
    networkManager(new QNetworkAccessManager(this)) {}

void PublicServers::getServers() {
    if (!serverAddress.isEmpty()) {
        emit serversReady(serverAddress, serverPort);
        return;
    }

    QUrl url(QString(PUBLIC_SERVERS_JSON));
    reply = networkManager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(serversDataDownloaded()));
}

void PublicServers::serversDataDownloaded() {
    reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    reply->deleteLater();
    if (errorCode != QNetworkReply::NoError) {
        qDebug() << "Error downloading public servers";
        return;
    }

    QJsonParseError parseError{};
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parsing error:" << parseError.errorString();
        return;
    }

    QVariantMap jsonMap = jsonResponse.toVariant().toMap();
    auto publicServersJSONList = jsonMap["wsamateur_servers"].toList();
    if (publicServersJSONList.empty()) {
        qDebug() << "Empty public server list";
        return;
    }

    const auto serverMap = publicServersJSONList.front().toMap();
    serverAddress = serverMap["host"].toString();
    serverPort = serverMap["port"].toString();
    emit serversReady(serverAddress, serverPort);
}
