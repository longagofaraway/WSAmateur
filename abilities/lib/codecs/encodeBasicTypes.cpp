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
    if (t.targetSpecification)
        encodeTargetSpecificCards(*t.targetSpecification, buf);
}

void encodeMultiplier(const Multiplier &m, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(m.type));
    if (m.forEach)
        encodeTarget(*m.forEach, buf);
    buf.push_back(static_cast<uint8_t>(m.zone));
}

void encodePlace(const Place &c, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(c.pos));
    buf.push_back(static_cast<uint8_t>(c.zone));
    buf.push_back(static_cast<uint8_t>(c.owner));
}

void encodeNumber(const Number &n, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(n.mod));
    buf.push_back(zzenc_8(n.value));
    if (n.multiplier)
        encodeMultiplier(*n.multiplier, buf);
}

void encodeString(const std::string &str, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(str.size()));
    buf.insert(buf.end(), str.begin(), str.end());
}
