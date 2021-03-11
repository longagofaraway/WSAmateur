#include "decode.h"

using namespace asn;

ConditionIsCard decodeConditionIsCard(Iterator &it, Iterator end) {
    ConditionIsCard c;
    c.target = decodeTarget(it, end);
    c.neededCard = decodeArray(it, end, decodeCard);
    return c;
}

ConditionHaveCard decodeConditionHaveCard(Iterator &it, Iterator end) {
    ConditionHaveCard c;
    c.invert = decodeBool(it, end);
    c.who = decodeEnum<Player>(it, end);
    c.howMany = decodeNumber(it, end);
    c.whichCards = decodeCard(it, end);
    c.where = decodePlace(it, end);
    c.excludingThis = decodeBool(it, end);
    return c;
}

ConditionCardsLocation decodeConditionCardsLocation(Iterator &it, Iterator end) {
    ConditionCardsLocation c;
    c.target = decodeTarget(it, end);
    c.place = decodePlace(it, end);
    return c;
}

template<typename T>
T decodeNumberCard(Iterator &it, Iterator end) {
    T t;
    t.number = decodeNumber(it, end);
    t.card = decodeCard(it, end);
    return t;
}

template<typename T>
T decodeConditionAndOr(Iterator &it, Iterator end) {
    T t;
    t.cond = decodeArray(it, end, decodeCondition);
    return t;
}

Condition decodeCondition(Iterator &it, Iterator end) {
    Condition c;

    c.type = decodeEnum<ConditionType>(it, end);
    switch (c.type) {
    case ConditionType::IsCard:
        c.cond = decodeConditionIsCard(it, end);
        break;
    case ConditionType::HaveCards:
        c.cond = decodeConditionHaveCard(it, end);
        break;
    case ConditionType::SumOfLevels:
        c.cond = ConditionSumOfLevels{ decodeUInt8(it, end) };
        break;
    case ConditionType::CardsLocation:
        c.cond = decodeConditionCardsLocation(it, end);
        break;
    case ConditionType::DuringTurn:
        c.cond = ConditionDuringTurn { decodeEnum<Owner>(it, end) };
        break;
    case ConditionType::CheckOpenedCards:
        c.cond = decodeNumberCard<ConditionCheckOpenedCards>(it, end);
        break;
    case ConditionType::RevealedCard:
        c.cond = decodeNumberCard<ConditionRevealCard>(it, end);
        break;
    case ConditionType::PlayersLevel:
        c.cond = ConditionPlayersLevel{ decodeNumber(it, end) };
        break;
    case ConditionType::And:
        c.cond = decodeConditionAndOr<ConditionAnd>(it, end);
        break;
    case ConditionType::Or:
        c.cond = decodeConditionAndOr<ConditionOr>(it, end);
        break;
    default:
        break;
    }

    return c;
}
