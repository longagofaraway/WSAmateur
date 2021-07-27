#include "decode.h"

using namespace asn;

TargetSpecificCards decodeTargetSpecificCards(Iterator &it, Iterator end) {
    TargetSpecificCards t;
    t.mode = decodeEnum<TargetMode>(it, end);
    t.number = decodeNumber(it, end);
    t.cards = decodeCard(it, end);
    return t;
}

Target decodeTarget(Iterator &it, Iterator end) {
    Target t;
    t.type = decodeEnum<TargetType>(it, end);
    if (t.type == TargetType::SpecificCards)
        t.targetSpecification = decodeTargetSpecificCards(it, end);
    return t;
}

std::string decodeString(Iterator &it, Iterator end) {
    checkDistance(it, end, 1);

    int size = *it++;
    checkDistance(it, end, static_cast<int>(size));

    std::string s(size, '\0');
    std::copy(it, it + size, s.begin());
    it += size;

    return s;
}

ForEachMultiplier decodeForEachMultiplier(Iterator &it, Iterator end) {
    ForEachMultiplier m;

    m.target = std::make_shared<Target>(decodeTarget(it, end));
    m.zone = decodeEnum<Zone>(it, end);

    return m;
}

Multiplier decodeMultiplier(Iterator &it, Iterator end) {
    Multiplier m;

    m.type = decodeEnum<MultiplierType>(it, end);
    if (m.type == MultiplierType::ForEach)
        m.specifier = decodeForEachMultiplier(it, end);

    return m;
}

Number decodeNumber(Iterator &it, Iterator end) {
    Number n;

    n.mod = decodeEnum<NumModifier>(it, end);
    n.value = decodeInt8(it, end);
    if (n.mod == NumModifier::Multiplier)
        n.multiplier = decodeMultiplier(it, end);

    return n;
}

Place decodePlace(Iterator &it, Iterator end) {
    Place p;

    p.pos = decodeEnum<Position>(it, end);
    p.zone = decodeEnum<Zone>(it, end);
    p.owner = decodeEnum<Player>(it, end);

    return p;
}
