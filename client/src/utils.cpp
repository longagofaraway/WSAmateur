#include "utils.h"

#include "codecs/decode.h"

asn::Ability decodeAbility(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.cbegin();
    return decodeAbility(it, binbuf.end());
}

void highlightAllCards(CardZone *zone, bool highlight) {
    for (int i = 0; i < zone->model().count(); ++i)
        zone->model().setGlow(i, highlight);
}
void selectAllCards(CardZone *zone, bool select) {
    for (int i = 0; i < zone->model().count(); ++i)
        zone->model().setSelected(i, select);
}

int highlightEligibleCards(CardZone *zone, const std::vector<asn::CardSpecifier> &specs,
                           asn::TargetMode mode, const ActivatedAbility &a) {
    auto noop = [](const Card&){ return true; };
    return highlightEligibleCards(zone, specs, mode, a, noop);
}
