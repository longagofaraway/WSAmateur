#include "encode.h"

using namespace asn;

void encodeConditionIsCard(const ConditionIsCard &c, Buf &buf) {
    encodeTarget(c.target, buf);
    encodeArray(c.neededCard, buf, encodeCard);
}

void encodeConditionHaveCard(const ConditionHaveCard &c, Buf &buf) {
    buf.push_back(c.invert ? 1 : 0);
    buf.push_back(static_cast<uint8_t>(c.who));
    encodeNumber(c.howMany, buf);
    encodeCard(c.whichCards, buf);
    encodePlace(c.where, buf);
    buf.push_back(c.excludingThis ? 1 : 0);
}

template<typename T>
void encodeConditionAndOr(const T &c, Buf &buf) {
    encodeArray(c.cond, buf, encodeCondition);
}

void encodeConditionCardsLocation(const ConditionCardsLocation &c, Buf &buf) {
    encodeTarget(c.target, buf);
    encodePlace(c.place, buf);
}

void encodeCondition(const Condition &c, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(c.type));
    switch (c.type) {
    case ConditionType::IsCard:
        encodeConditionIsCard(std::get<ConditionIsCard>(c.cond), buf);
        break;
    case ConditionType::HaveCards:
        encodeConditionHaveCard(std::get<ConditionHaveCard>(c.cond), buf);
        break;
    case ConditionType::And:
        encodeConditionAndOr(std::get<ConditionAnd>(c.cond), buf);
        break;
    case ConditionType::Or:
        encodeConditionAndOr(std::get<ConditionOr>(c.cond), buf);
        break;
    case ConditionType::SumOfLevels:
        buf.push_back(static_cast<uint8_t>(std::get<ConditionSumOfLevels>(c.cond).equalOrMoreThan));
        break;
    case ConditionType::CardsLocation:
        encodeConditionCardsLocation(std::get<ConditionCardsLocation>(c.cond), buf);
        break;
    case ConditionType::DuringTurn:
        buf.push_back(static_cast<uint8_t>(std::get<ConditionDuringTurn>(c.cond).player));
        break;
    case ConditionType::CheckMilledCards:
        encodeNumberCard(std::get<ConditionCheckMilledCards>(c.cond), buf);
        break;
    case ConditionType::RevealedCard:
        encodeNumberCard(std::get<ConditionRevealCard>(c.cond), buf);
        break;
    case ConditionType::PlayersLevel:
        encodeNumber(std::get<ConditionPlayersLevel>(c.cond).value, buf);
        break;
    default:
        break;
    }
}
