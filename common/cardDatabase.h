#pragma once

#include <unordered_map>

#include "cardInfo.h"

class CardDatabase
{
    std::unordered_map<std::string, std::shared_ptr<CardInfo>> mDb;

    CardDatabase();
public:
    static CardDatabase& get();

    std::shared_ptr<CardInfo> getCard(const std::string &code);
};
