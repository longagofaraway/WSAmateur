#include "abilityPlayer.h"

#include "abilityUtils.h"
#include "serverPlayer.h"
#include "serverGame.h"

Resumable AbilityPlayer::payCost() {
    if (!hasCost())
        co_return;

    for (const auto &item: cost().items) {
        if (item.type == asn::CostType::Stock) {
            for (int i = 0; i < std::get<asn::StockCost>(item.costItem).value; ++i)
                mPlayer->moveCard("stock", mPlayer->zone("stock")->count() - 1, "wr");
        } else {
            co_await playEffect(std::get<asn::Effect>(item.costItem));
        }
    }
}

Resumable AbilityPlayer::playEventAbility(const asn::EventAbility &a) {
    co_await playEffects(a.effects);
}

Resumable AbilityPlayer::playAutoAbility(const asn::AutoAbility &a) {
    if (a.cost)
        setCost(*a.cost);
    co_await playEffects(a.effects);
}

Resumable AbilityPlayer::playActAbility(const asn::ActAbility &a) {
    setCost(a.cost);
    co_await payCost();
    co_await playEffects(a.effects);
}

void AbilityPlayer::playContAbility(const asn::ContAbility &a, bool &active) {
    if (a.effects.size() == 0)
        return;

    bool res = evaluateCondition(a.effects[0].cond);
    if (!res && !active)
        return;
    if (!res && active) {
        setRevert(true);
        active = false;
    } else {
        active = true;
    }
    for (const auto &effect: a.effects)
        playContEffect(effect);
}

void AbilityPlayer::revertContAbility(const asn::ContAbility &a) {
    if (a.effects.size() == 0)
        return;

    setRevert(true);
    for (const auto &effect: a.effects)
        playContEffect(effect);
}

Resumable AbilityPlayer::playAbility(const asn::Ability &a) {
    switch(a.type) {
    case asn::AbilityType::Auto:
        co_await playAutoAbility(std::get<asn::AutoAbility>(a.ability));
        break;
    case asn::AbilityType::Event:
        co_await playEventAbility(std::get<asn::EventAbility>(a.ability));
        break;
    case asn::AbilityType::Act:
        co_await playActAbility(std::get<asn::ActAbility>(a.ability));
        break;
    default:
        break;
    }
}

ServerPlayer* AbilityPlayer::owner(asn::Player player) const {
    assert(player != asn::Player::Both && player != asn::Player::NotSpecified);
    return player == asn::Player::Player ? mPlayer : mPlayer->game()->opponentOfPlayer(mPlayer->id());
}

ServerPlayer* AbilityPlayer::owner(ServerCard *card) const {
    bool player = card->zone()->player() == mPlayer;
    return player ? mPlayer : mPlayer->game()->opponentOfPlayer(mPlayer->id());
}

void AbilityPlayer::removeMentionedCard(int cardId) {
    std::erase_if(mMentionedCards, [cardId](CardImprint &im) { return cardId == im.card->id(); });
}

int AbilityPlayer::getForEachMultiplierValue(const asn::Multiplier &m) {
    assert(m.forEach->type == asn::TargetType::SpecificCards);
    auto pzone = mPlayer->zone(asnZoneToString(m.zone));
    int cardCount = 0;
    for (int i = 0; i < pzone->count(); ++i) {
        auto card = pzone->card(i);
        if (!card)
            continue;

        const auto &tspec = m.forEach->targetSpecification;

        if (!checkTargetMode(tspec->mode, thisCard().card, card))
            continue;

        if (checkCard(tspec->cards.cardSpecifiers, *card))
            cardCount++;
    }

    return cardCount;
}

std::vector<ServerCard*> AbilityPlayer::getTargets(const asn::Target &t) {
    std::vector<ServerCard*> targets;
    if (t.type == asn::TargetType::ChosenCards) {
        for (const auto &card: chosenCards())
            targets.push_back(card.card);
    } else if (t.type == asn::TargetType::MentionedCards) {
        for (const auto &card: mentionedCards())
            targets.push_back(card.card);
    } else if (t.type == asn::TargetType::LastMovedCards) {
        for (const auto &card: lastMovedCards())
            targets.push_back(card.card);
    } else if (t.type == asn::TargetType::ThisCard) {
        if (thisCard().card == nullptr)
            return targets;
        targets.push_back(thisCard().card);
    } else if (t.type == asn::TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        auto stage = mPlayer->zone("stage");
        for (int i = 0; i < stage->count(); ++i) {
            auto card = stage->card(i);
            if (!card)
                continue;

            if (!checkTargetMode(spec.mode, thisCard().card, card))
                continue;

            if (checkCard(spec.cards.cardSpecifiers, *card))
                targets.push_back(card);
        }
    } else if (t.type == asn::TargetType::OppositeThis) {
        auto card = mPlayer->oppositeCard(thisCard().card);
        if (card)
            targets.push_back(card);
    }
    return targets;
}
