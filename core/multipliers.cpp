#include "abilityPlayer.h"

#include <numeric>

#include "serverPlayer.h"


int AbilityPlayer::getForEachMultiplierValue(const asn::Multiplier &m) {
    const auto &specifier = std::get<asn::ForEachMultiplier>(m.specifier);
    assert(specifier.target->type == asn::TargetType::SpecificCards);

    int cardCount = 0;
    const auto &tspec = specifier.target->targetSpecification.value();

    if (specifier.placeType == asn::PlaceType::SpecificPlace) {
        auto player = owner(specifier.place->owner);
        auto pzone = player->zone(specifier.place->zone);
        for (int i = 0; i < pzone->count(); ++i) {
            if (checkTarget(tspec, pzone->card(i)))
                cardCount++;
        }
    } else if (specifier.placeType == asn::PlaceType::LastMovedCards) {
        for (const auto &card: lastMovedCards()) {
            if (checkTarget(tspec, card.card))
                cardCount++;
        }
    } else if (specifier.placeType == asn::PlaceType::Selection) {
        for (const auto &card: mentionedCards()) {
            if (checkTarget(tspec, card.card))
                cardCount++;
        }
    } else if (specifier.placeType == asn::PlaceType::Marker) {
        if (!specifier.markerBearer.has_value() || !specifier.markerBearer.value())
            return cardCount;

        const auto &markerBearer = *specifier.markerBearer.value();
        auto bearers = getTargets(markerBearer);
        for (const auto target: bearers) {
            if (!target) continue;
            for (const auto &marker: target->markers()) {
                if (checkTarget(tspec, marker.get()))
                    cardCount++;
            }
        }
    }

    return cardCount;
}

int AbilityPlayer::getAddLevelMultiplierValue(const asn::Multiplier &m) {
    const auto &specifier = std::get<asn::AddLevelMultiplier>(m.specifier);
    auto targets = getTargets(*specifier.target);
    int res = 0;
    for (const auto &target: targets) {
        res += target->level();
    }
    return res;
}

int AbilityPlayer::getTriggerNumberMultiplierValue(const asn::Multiplier &m) {
    const auto &specifier = std::get<asn::AddTriggerNumberMultiplier>(m.specifier);
    auto targets = getTargets(*specifier.target);
    return std::accumulate(targets.begin(), targets.end(), 0, [&specifier](int sum, const ServerCard *card){
        const auto &triggerIcons = card->triggers();
        return sum + std::accumulate(triggerIcons.begin(), triggerIcons.end(), 0, [&specifier](int sum, asn::TriggerIcon icon){
            return icon == specifier.triggerIcon ? sum + 1 : sum;
        });
    });
}
