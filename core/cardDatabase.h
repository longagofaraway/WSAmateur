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

    CardDatabase();
    CardDatabase(const CardDatabase&) = delete;
    CardDatabase& operator=(const CardDatabase&) = delete;
public:
    static CardDatabase& get();

    std::shared_ptr<CardInfo> getCard(const std::string &code);

private:
    void fillCache();
};
