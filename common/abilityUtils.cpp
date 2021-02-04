#include "abilityUtils.h"

#include <cassert>

#include <QByteArray>

#include "abilities.pb.h"

std::string_view asnZoneToString(asn::Zone zone) {
    switch (zone) {
    case asn::Zone::Climax:
        return "climax";
    case asn::Zone::Clock:
        return "clock";
    case asn::Zone::Deck:
        return "deck";
    case asn::Zone::Hand:
        return "hand";
    case asn::Zone::Level:
        return "level";
    case asn::Zone::Memory:
        return "memory";
    case asn::Zone::Stage:
        return "stage";
    case asn::Zone::Stock:
        return "stock";
    case asn::Zone::WaitingRoom:
        return "wr";
    default:
        assert(false);
        return "";
    }
}

QString asnZoneToReadableString(asn::Zone zone) {
    switch (zone) {
    case asn::Zone::Climax:
        return "Climax";
    case asn::Zone::Clock:
        return "Clock";
    case asn::Zone::Deck:
        return "Deck";
    case asn::Zone::Hand:
        return "Hand";
    case asn::Zone::Level:
        return "Level";
    case asn::Zone::Memory:
        return "Memory";
    case asn::Zone::Stage:
        return "Stage";
    case asn::Zone::Stock:
        return "Stock";
    case asn::Zone::WaitingRoom:
        return "Waiting Room";
    default:
        assert(false);
        return "";
    }
}

uint32_t abilityHash(const ProtoAbility &a) {
    std::string buf = a.zone();
    buf += a.cardcode();
    buf += std::to_string(a.cardid());
    buf += std::to_string(a.type());
    buf += std::to_string(a.abilityid());
    return qChecksum(buf.data(), static_cast<uint>(buf.size()));
}
