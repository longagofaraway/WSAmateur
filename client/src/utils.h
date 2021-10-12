#pragma once

#include "abilities.h"
#include "abilityUtils.h"
#include "activatedAbilities.h"
#include "cardZone.h"

struct ActivatedAbility;
class CardZone;

asn::Ability decodeAbility(const std::string &buf);
void highlightAllCards(CardZone *zone, bool highlight);
void selectAllCards(CardZone *zone, bool select);
int getForEachMultiplierValue(Player *player, int thisCardId, const asn::Multiplier &multiplier);

template<typename T>
int highlightEligibleCards(CardZone *zone, const std::vector<asn::CardSpecifier> &specs,
                             asn::TargetMode mode, const ActivatedAbility &a, T pred) {
    int eligibleCount = 0;
    const auto &cards = zone->cards();
    highlightAllCards(zone, false);
    selectAllCards(zone, false);

    for (int i = 0; i < zone->model().count(); ++i) {
        if (!cards[i].cardPresent())
            continue;

        if ((mode == asn::TargetMode::FrontRow && i > 2) ||
            (mode == asn::TargetMode::BackRow && i < 3))
            continue;

        if (mode == asn::TargetMode::AllOther &&
            a.zone == zone->name() && a.cardId == cards[i].id())
            continue;

        if (!pred(cards[i]))
            continue;

        if (checkCard(specs, cards[i])) {
            zone->model().setGlow(i, true);
            eligibleCount++;
        }
    }

    return eligibleCount;
}

int highlightEligibleCards(CardZone *zone, const std::vector<asn::CardSpecifier> &specs,
                           asn::TargetMode mode, const ActivatedAbility &a);

