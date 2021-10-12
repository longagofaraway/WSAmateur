#include "utils.h"

#include "codecs/decode.h"
#include "player.h"

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


namespace {
bool isInFrontOf(int backPos, int frontPos) {
    if ((backPos == 3 && (frontPos == 0 || frontPos == 1))
        || (backPos == 4 && (frontPos == 1 || frontPos == 2)))
        return true;
    return false;
}
bool isBackRow(int pos) {
    return pos == 3 || pos == 4;
}
bool isFrontRow(int pos) {
    return pos >= 0 && pos <= 2;
}

bool checkTargetMode(asn::TargetMode mode, const Card *thisCard, int thisCardPos,
                     const Card *card, int cardPos) {
    if (mode == asn::TargetMode::AllOther ||
        mode == asn::TargetMode::BackRowOther ||
        mode == asn::TargetMode::FrontRowOther) {
        if (!thisCard)
            return true;
        if (!thisCard->id())
            return true;
    }
    switch (mode) {
    case asn::TargetMode::AllOther:
        return thisCard->id() != card->id();
    case asn::TargetMode::InFrontOfThis:
        if (! thisCard || thisCardPos < 0)
            return false;
        return isInFrontOf(thisCardPos, cardPos);
    case asn::TargetMode::BackRow:
        return isBackRow(cardPos);
    case asn::TargetMode::FrontRow:
        return isFrontRow(cardPos);
    case asn::TargetMode::BackRowOther:
        return isBackRow(cardPos) && thisCard->id() != card->id();
    case asn::TargetMode::FrontRowOther:
        return isFrontRow(cardPos) && thisCard->id() != card->id();
    }

    return true;
}
}

int getForEachMultiplierValue(Player *player, int thisCardId, const asn::Multiplier &multiplier) {
    auto &specifier = *multiplier.specifier;
    assert(specifier.target->type == asn::TargetType::SpecificCards);
    auto pzone = player->zone(asnZoneToString(specifier.zone));
    auto thisCard = pzone->findCardById(thisCardId);
    int thisCardPos = pzone->model().findById(thisCardId);
    int cardCount = 0;
    for (int i = 0; i < pzone->model().count(); ++i) {
        auto &card = pzone->cards()[i];
        if (!card.cardPresent())
            continue;

        const auto &tspec = specifier.target->targetSpecification;

        if (!checkTargetMode(tspec->mode, thisCard, thisCardPos, &card, i))
            continue;

        if (checkCard(tspec->cards.cardSpecifiers, card))
            cardCount++;
    }

    return cardCount;
}
