#include "imageLinks.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include "filesystemPaths.h"

ImageLinks::ImageLinks() {}

ImageLinks& ImageLinks::get() {
    static ImageLinks instance;
    return instance;
}

std::optional<QString> ImageLinks::imageLink(const std::string &code) {
    if (!cardImageLinks.contains(code))
        return {};
    return cardImageLinks.at(code);
}

bool ImageLinks::setData(QString filePath) {
    QByteArray data;
    if (!loadData(filePath, data))
        return false;

    QFile localImageFile(paths::imageLinksPath());
    if (!localImageFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    localImageFile.write(data);
    localImageFile.close();
    return true;
}

bool ImageLinks::loadFile(QString filePath) {
    QByteArray data;
    return loadData(filePath, data);
}

bool ImageLinks::loadData(QString filePath, QByteArray &data) {
    QString parsedFilePath = filePath;
    if (parsedFilePath.startsWith("file:")) {
        QUrl fileUrl(filePath);
        parsedFilePath = fileUrl.toLocalFile();
    }
    QFile file(parsedFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    data = file.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return false;

    if (!jsonResponse.isObject())
        return false;

    auto jsonObject = jsonResponse.object();
    if (!jsonObject.contains("version") || !jsonObject.contains("urls") ||
            !jsonObject["urls"].isArray())
        return false;

    version_ = jsonObject["version"].toInt();
    auto array = jsonObject["urls"].toArray();
    for (int i = 0; i < array.size(); ++i) {
        auto object = array[i].toObject();
        if (!object.contains("code") || !object.contains("url"))
            return false;
        cardImageLinks.emplace(object["code"].toString().toStdString(),
                               object["url"].toString());
    }
    return true;
}

bool ImageLinks::update(const std::string &newData) {
    QFile file(paths::imageLinksPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    QByteArray data(newData.data(), static_cast<int>(newData.size()));
    int written = file.write(data);
    file.close();

    if (written == -1)
        return false;

    QByteArray buf;
    return loadData(paths::imageLinksPath(), buf);
}
