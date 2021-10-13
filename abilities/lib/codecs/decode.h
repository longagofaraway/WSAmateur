#pragma once

#include <stdexcept>

#include "abilities.h"
#include "encDecUtils.h"

using Iterator = std::vector<uint8_t>::const_iterator;

class DecodeException : public std::runtime_error
{
public:
    DecodeException() : std::runtime_error("") {}
    DecodeException(const char * what_arg) : std::runtime_error(what_arg) {}
};

inline void checkDistance(Iterator start, Iterator end, int needed_length) {
    if (std::distance(start, end) < needed_length)
        throw DecodeException("unexpected end of file");
}

template<typename T>
std::vector<T> decodeArray(Iterator &it, Iterator end, T(*decodeType)(Iterator&, Iterator)) {
    checkDistance(it, end, 1);
    size_t size = *it++;
    std::vector<T> vec;
    for (size_t i = 0; i < size; ++i)
        vec.push_back(decodeType(it, end));

    return vec;
}

template<typename T>
T decodeEnum(Iterator &it, Iterator end) {
    checkDistance(it, end, 1);

    return static_cast<T>(*it++);
}

template<typename T>
T fromLE(Iterator &it) {
    T val = 0;
    for (int i = 0; i < static_cast<int>(sizeof(T)); ++i, ++it)
        val += (*it << (i * 8));

    return val;
}
inline int8_t decodeInt8(Iterator &it, Iterator end) {
    checkDistance(it, end, 1);
    return zzdec_8(*it++);
}
inline uint8_t decodeUInt8(Iterator &it, Iterator end) {
    checkDistance(it, end, 1);
    return *it++;
}
inline int32_t decodeInt32(Iterator &it, Iterator end) {
    checkDistance(it, end, 4);
    return zzdec_32(fromLE<uint32_t>(it));
}
inline bool decodeBool(Iterator &it, Iterator end) {
    checkDistance(it, end, 1);
    return (*it++) == 0 ? false : true;
}

asn::Ability decodeAbility(Iterator &it, Iterator end);
asn::EventAbility decodeEventAbility(Iterator &it, Iterator end);
asn::Effect decodeEffect(Iterator &it, Iterator end);
asn::Trigger decodeTrigger(Iterator &it, Iterator end);
std::string decodeString(Iterator &it, Iterator end);
asn::Number decodeNumber(Iterator &it, Iterator end);
asn::Card decodeCard(Iterator &it, Iterator end);
asn::Target decodeTarget(Iterator &it, Iterator end);
asn::Place decodePlace(Iterator &it, Iterator end);
asn::Condition decodeCondition(Iterator &it, Iterator end);
asn::Multiplier decodeMultiplier(Iterator &it, Iterator end);

asn::ChooseCard decodeChooseCard(Iterator &it, Iterator end);
asn::MoveCard decodeMoveCard(Iterator &it, Iterator end);
asn::DrawCard decodeDrawCard(Iterator &it, Iterator end);
asn::SearchCard decodeSearchCard(Iterator &it, Iterator end);
asn::AbilityGain decodeAbilityGain(Iterator &it, Iterator end);
asn::PerformEffect decodePerformEffect(Iterator &it, Iterator end);
asn::Look decodeLook(Iterator &it, Iterator end);
asn::RevealCard decodeRevealCard(Iterator &it, Iterator end);
asn::ChangeState decodeChangeState(Iterator &it, Iterator end);
