#pragma once

#include <string>

class ServerCard;

struct CardImprint {
    ServerCard *card;
    std::string zone;
    int id = 0;
    bool opponent = false;
    CardImprint() = default;
    CardImprint(const std::string &zone, int id, ServerCard *card = nullptr, bool opponent = false)
        : card(card), zone(zone), id(id), opponent(opponent) {}
};
