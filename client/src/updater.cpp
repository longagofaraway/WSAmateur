#include "updater.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>

#include "filesystemPaths.h"
#include "versionParser.h"

namespace {
const QString kReleaseUrl = "https://api.github.com/repos/longagofaraway/WSAmateur/releases";
}

Updater::Updater(QObject *parent, const std::string &neededVersion_)
    : QObject(parent) {
    neededVersion = QString::fromStdString(neededVersion_);
    networkManager = new QNetworkAccessManager(this);
}

void Updater::startUpdate() {
    response = networkManager->get(QNetworkRequest(kReleaseUrl));
    connect(response, SIGNAL(finished()), this, SLOT(releaseListReady()));
}

void Updater::releaseListReady() {
    auto *reply = static_cast<QNetworkReply *>(sender());
    QJsonParseError parseError{};
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
    reply->deleteLater();

    if (parseError.error != QJsonParseError::NoError) {
        emit error("No reply received from the release update server.");
        return;
    }

    QJsonArray array = jsonResponse.array();
    if (array.empty()) {
        emit error("No reply received from the release update server.");
        return;
    }

    bool releaseMatched = false;
    for (auto obj: array) {
        try {
        if (parseRelease(obj.toObject().toVariantMap())) {
            releaseMatched = true;
            break;
        }
        } catch (const std::exception&) {
            continue;
        }
    }

    if (releaseMatched && !downloadUrl.isEmpty()) {
        downloadInstaller(downloadUrl);
    }
}

void Updater::installerDownloaded() {
    auto *reply = static_cast<QNetworkReply *>(sender());

    QVariant redirect = response->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirect.isNull()) {
        reply->deleteLater();
        downloadInstaller(redirect.toUrl());
        return;
    }

    if (reply->error()) {
        emit error(response->errorString());
        reply->deleteLater();
        return;
    }


    auto tmpDir = paths::tempUpdaterPath();
    QString fileName = tmpDir.filePath(downloadUrl.section('/', -1));

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("Could not open the file for reading.");
        return;
    }

    file.write(response->readAll());
    file.close();

    reply->deleteLater();

    QProcess process;
    process.setProgram("cmd.exe");
    process.setArguments({"/C", paths::updateScriptFullPath(), QCoreApplication::applicationDirPath() + "/WSAmateur"});
    process.setWorkingDirectory(paths::updateDirPath().path());
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    process.startDetached();

    QCoreApplication::quit();
}

void Updater::downloadProgress(qint64 bytesRead, qint64 totalBytes) {
    emit progressMade(bytesRead, totalBytes);
}

bool Updater::parseRelease(QVariantMap release) {
    if (release.empty()) {
        emit error("No reply received from the release update server.");
        return false;
    }

    if (!release.contains("assets") || !release.contains("tag_name") ||
        !release.contains("published_at")) {
        qWarning() << "Invalid reply received from the release update server:" << release;
        emit error("Invalid reply received from the release update server.");
        return false;
    }

    QString tagName = release["tag_name"].toString();
    if (tagName.isEmpty()) {
        emit error("Error pasrsing tag_name - empty tag_name");
        return false;
    }
    //remove 'v' at the beginning
    tagName.remove(0, 1);
    auto releaseVersion = parseVersion(tagName);
    auto clientVersion = parseVersion(neededVersion);

    if (releaseVersion[1] != clientVersion[1] ||
        releaseVersion[0] != clientVersion[0]) {
        return false;
    }

    auto rawAssets = release["assets"].toList();
    QVariantMap asset = rawAssets.at(0).toMap();
    downloadUrl = asset["browser_download_url"].toString();

    return true;
}

void Updater::downloadInstaller(QUrl url) {
    response = networkManager->get(QNetworkRequest(url));
    connect(response, SIGNAL(finished()), this, SLOT(installerDownloaded()));
    connect(response, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    //connect(this, SIGNAL(stopDownload()), response, SLOT(abort()));
}
