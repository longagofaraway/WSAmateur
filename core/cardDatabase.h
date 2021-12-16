#pragma once

#include <memory>
#include <unordered_map>

#include <QSqlDatabase>
#include <QString>

#include "cardInfo.h"

class CardDatabase
{
    QSqlDatabase db;
    std::unordered_map<std::string, std::shared_ptr<CardInfo>> cards;
    bool versionCached = false;
    int version_;
    bool initialized_;

    CardDatabase();
    CardDatabase(const CardDatabase&) = delete;
    CardDatabase& operator=(const CardDatabase&) = delete;
public:
    static CardDatabase& get();

    void init();
    bool initialized() const { return initialized_; }
    std::shared_ptr<CardInfo> getCard(const std::string &code) const;
    int version() const;
    void update(const std::string &newDb);
    std::string fileData() const;

private:
    void fillCache();
    void readVersion();
};
