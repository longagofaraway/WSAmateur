#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include <QString>

class ImageLinks
{
    int version_ = 0;
    std::unordered_map<std::string, QString> cardImageLinks;

    ImageLinks();
    ImageLinks(const ImageLinks&) = delete;
    ImageLinks& operator=(const ImageLinks&) = delete;

public:
    static ImageLinks& get();

    std::optional<QString> imageLink(const std::string& code);
    bool setData(QString filePath);
    bool loadFile(QString filePath);
    bool update(const std::string &data);
    int version() const { return version_; }

private:
    bool loadData(QString filePath, QByteArray &data);
};

