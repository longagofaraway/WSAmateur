#include "encode.h"

#include "encDecUtils.h"

using namespace asn;

void encodeTargetSpecificCards(const TargetSpecificCards &t, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(t.mode));
    encodeNumber(t.number, buf);
    encodeCard(t.cards, buf);
}

void encodeTarget(const Target &t, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(t.type));
    if (t.type == TargetType::SpecificCards ||
        t.type == TargetType::BattleOpponent)
        encodeTargetSpecificCards(t.targetSpecification.value(), buf);
}

void encodeForEachMultiplier(const ForEachMultiplier &m, Buf &buf) {
    encodeTarget(*m.target, buf);
    buf.push_back(static_cast<uint8_t>(m.zone));
}

void encodeMultiplier(const Multiplier &m, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(m.type));
    if (m.type == MultiplierType::ForEach && m.specifier)
        encodeForEachMultiplier(*m.specifier, buf);
}

void encodePlace(const Place &c, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(c.pos));
    buf.push_back(static_cast<uint8_t>(c.zone));
    buf.push_back(static_cast<uint8_t>(c.owner));
}

void encodeNumber(const Number &n, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(n.mod));
    buf.push_back(zzenc_8(n.value));
}

void encodeString(const std::string &str, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(str.size()));
    buf.insert(buf.end(), str.begin(), str.end());
}
