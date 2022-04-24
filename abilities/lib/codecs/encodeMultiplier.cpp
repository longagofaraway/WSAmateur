#include "encode.h"

using namespace asn;

void encodeForEachMultiplier(const ForEachMultiplier &m, Buf &buf) {
    encodeTarget(*m.target, buf);
    buf.push_back(static_cast<uint8_t>(m.placeType));
    if (m.placeType == PlaceType::SpecificPlace)
        encodePlace(m.place.value(), buf);
}

void encodeAddLevelMultiplier(const AddLevelMultiplier &m, Buf &buf) {
    encodeTarget(*m.target, buf);
}

void encodeAddTriggerNumberMultiplier(const AddTriggerNumberMultiplier &m, Buf &buf) {
    encodeTarget(*m.target, buf);
    buf.push_back(static_cast<uint8_t>(m.triggerIcon));
}

void encodeMultiplier(const Multiplier &m, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(m.type));
    if (m.type == MultiplierType::ForEach)
        encodeForEachMultiplier(std::get<ForEachMultiplier>(m.specifier), buf);
    else if (m.type == MultiplierType::AddLevel)
        encodeAddLevelMultiplier(std::get<AddLevelMultiplier>(m.specifier), buf);
    else if (m.type == MultiplierType::AddTriggerNumber)
        encodeAddTriggerNumberMultiplier(std::get<AddTriggerNumberMultiplier>(m.specifier), buf);
}
