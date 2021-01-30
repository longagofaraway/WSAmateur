#pragma once

#include "abilities.h"

using Buf = std::vector<uint8_t>;

inline void toBufLE(uint32_t val, Buf &buf) {
    for (int i = 0; i < sizeof(val); ++i)
        buf.push_back((val >> (i * 8)) & 0xFF);
}

void encodeAbility(const Ability &a, Buf &buf);
void encodeTrigger(const Trigger &t, Buf &buf);
void encodeTarget(const Target &t, Buf &buf);
void encodeNumber(const Number &n, Buf &buf);
void encodePlace(const Place &c, Buf &buf);
void encodeString(const std::string &str, Buf &buf);
void encodeCard(const AsnCard &t, Buf &buf);
void encodeEffect(const Effect &t, Buf &buf);
void encodeCondition(const Condition &c, Buf &buf);
void encodeMultiplier(const Multiplier &m, Buf &buf);

template<typename T>
void encodeArray(const std::vector<T> &array, Buf &buf, void(*encodeType)(const T&, Buf&)) {
    buf.push_back(static_cast<uint8_t>(array.size()));
    for (const auto &el: array)
        encodeType(el, buf);
}

template<typename T>
void encodeNumberCard(const T &t, Buf &buf) {
    encodeNumber(t.number, buf);
    encodeCard(t.card, buf);
}
