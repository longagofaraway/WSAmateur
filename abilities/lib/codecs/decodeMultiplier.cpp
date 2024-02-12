#include "decode.h"

using namespace asn;

ForEachMultiplier decodeForEachMultiplier(Iterator &it, Iterator end) {
    ForEachMultiplier m;

    m.target = std::make_shared<Target>(decodeTarget(it, end));
    m.placeType = decodeEnum<PlaceType>(it, end);
    if (m.placeType == PlaceType::SpecificPlace)
        m.place = decodePlace(it, end);
    if (m.placeType == PlaceType::Marker)
        m.markerBearer = std::make_shared<Target>(decodeTarget(it, end));

    return m;
}

AddLevelMultiplier decodeAddLevelMultiplier(Iterator &it, Iterator end) {
    AddLevelMultiplier m;
    m.target = std::make_shared<Target>(decodeTarget(it, end));
    return m;
}

AddTriggerNumberMultiplier decodeAddTriggerNumberMultiplier(Iterator &it, Iterator end) {
    AddTriggerNumberMultiplier m;
    m.target = std::make_shared<Target>(decodeTarget(it, end));
    m.triggerIcon = decodeEnum<TriggerIcon>(it, end);
    return m;
}

Multiplier decodeMultiplier(Iterator &it, Iterator end) {
    Multiplier m;

    m.type = decodeEnum<MultiplierType>(it, end);
    if (m.type == MultiplierType::ForEach)
        m.specifier = decodeForEachMultiplier(it, end);
    else if (m.type == MultiplierType::AddLevel)
        m.specifier = decodeAddLevelMultiplier(it, end);
    else if (m.type == MultiplierType::AddTriggerNumber)
        m.specifier = decodeAddTriggerNumberMultiplier(it, end);

    return m;
}
