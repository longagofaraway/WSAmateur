#pragma once

#include <memory>
#include <unordered_map>

#include "cardInfo.h"

class CardDatabase
{
    std::unordered_map<std::string, std::shared_ptr<CardInfo>> mDb;

    CardDatabase();
    CardDatabase(const CardDatabase&) = delete;
    CardDatabase& operator=(const CardDatabase&) = delete;
public:
    static CardDatabase& get();

    std::shared_ptr<CardInfo> getCard(const std::string &code);
};
