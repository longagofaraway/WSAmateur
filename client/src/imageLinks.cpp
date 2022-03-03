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
    QString parsedFilePath = filePath;
    if (parsedFilePath.startsWith("file:")) {
        QUrl fileUrl(filePath);
        parsedFilePath = fileUrl.toLocalFile();
    }
    QFile file(parsedFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    auto data = file.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return false;

    if (!jsonResponse.isArray())
        return false;

    auto array = jsonResponse.array();
    for (int i = 0; i < array.size(); ++i) {
        auto object = array[i].toObject();
        if (!object.contains("code") || !object.contains("url"))
            return false;
        cardImageLinks.emplace(object["code"].toString().toStdString(),
                               object["url"].toString());
    }

    QFile localImageFile(paths::imageLinksPath());
    if (!localImageFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    localImageFile.write(data);
    localImageFile.close();
    return true;
}
