#pragma once

#include <string>

class ServerCard;

struct CardImprint {
    ServerCard *card;
    std::string zone;
    bool opponent = false;
    CardImprint() = default;
    CardImprint(const std::string &zone, ServerCard *card = nullptr, bool opponent = false)
        : card(card), zone(zone), opponent(opponent) {}
};
