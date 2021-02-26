#pragma once

#include "abilities.h"

using Buf = std::vector<uint8_t>;

inline void toBufLE(uint32_t val, Buf &buf) {
    for (int i = 0; i < sizeof(val); ++i)
        buf.push_back((val >> (i * 8)) & 0xFF);
}

void encodeAbility(const asn::Ability &a, Buf &buf);
void encodeTrigger(const asn::Trigger &t, Buf &buf);
void encodeTarget(const asn::Target &t, Buf &buf);
void encodeNumber(const asn::Number &n, Buf &buf);
void encodePlace(const asn::Place &c, Buf &buf);
void encodeString(const std::string &str, Buf &buf);
void encodeCard(const asn::Card &t, Buf &buf);
void encodeEffect(const asn::Effect &t, Buf &buf);
void encodeCondition(const asn::Condition &c, Buf &buf);
void encodeMultiplier(const asn::Multiplier &m, Buf &buf);

void encodeChooseCard(const asn::ChooseCard &m, Buf &buf);
void encodeMoveCard(const asn::MoveCard &e, Buf &buf);
void encodeDrawCard(const asn::DrawCard &e, Buf &buf);
void encodeSearchCard(const asn::SearchCard &e, Buf &buf);
void encodeAbilityGain(const asn::AbilityGain &e, Buf &buf);

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
