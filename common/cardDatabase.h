#pragma once

#include <unordered_map>

#include "cardInfo.h"

class CardDatabase
{
    std::unordered_map<std::string, CardInfo> mDb;

    CardDatabase();
public:
    static CardDatabase& get();

    CardInfo getCard(const std::string &code);
};
