#include "abilityPlayer.h"

#include "abilityUtils.h"
#include "serverPlayer.h"
#include "serverGame.h"

Resumable AbilityPlayer::payCost() {
    if (!hasCost())
        co_return;

    setPayingCost(true);
    for (const auto &item: cost().items) {
        if (item.type == asn::CostType::Stock) {
            for (int i = 0; i < std::get<asn::StockCost>(item.costItem).value; ++i)
                mPlayer->moveCard("stock", mPlayer->zone("stock")->count() - 1, "wr");
        } else {
            co_await playEffect(std::get<asn::Effect>(item.costItem));
        }
    }
    setPayingCost(false);
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

    setCont(true);
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

    setCont(true);
    setRevert(true);
    for (const auto &effect: a.effects)
        playContEffect(effect);
}

Resumable AbilityPlayer::playAbility(const asn::Ability a) {
    setCont(false);
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

void AbilityPlayer::setThisCard(ServerCard *card) {
    mThisCard = CardImprint(card->zone()->name(), card);
}

void AbilityPlayer::removeMentionedCard(int cardId) {
    std::erase_if(mMentionedCards, [cardId](CardImprint &im) { return cardId == im.card->id(); });
}

int AbilityPlayer::getForEachMultiplierValue(const asn::Multiplier &m) {
    auto &specifier = m.specifier.value();
    assert(specifier.target->type == asn::TargetType::SpecificCards);

    auto checkTargetCard = [this](const asn::TargetSpecificCards &spec, ServerCard *card) {
        if (!card)
            return false;

        if (!checkTargetMode(spec.mode, thisCard().card, card))
            return false;

        return checkCard(spec.cards.cardSpecifiers, *card);
    };

    int cardCount = 0;
    const auto &tspec = specifier.target->targetSpecification.value();

    if (specifier.placeType == asn::PlaceType::SpecificPlace) {
        auto pzone = mPlayer->zone(specifier.place->zone);
        for (int i = 0; i < pzone->count(); ++i) {
            if (checkTargetCard(tspec, pzone->card(i)))
                cardCount++;
        }
    } else if (specifier.placeType == asn::PlaceType::LastMovedCards) {
        for (const auto &card: lastMovedCards()) {
            if (checkTargetCard(tspec, card.card))
                cardCount++;
        }
    }

    return cardCount;
}

std::vector<ServerCard*> AbilityPlayer::getTargets(const asn::Target &t, asn::Zone from_zone) {
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
        auto stage = mPlayer->zone(from_zone);
        for (int i = 0; i < stage->count(); ++i) {
            auto card = stage->card(i);
            if (!card)
                continue;

            if (!checkTargetMode(spec.mode, thisCard().card, card))
                continue;

            if (checkCard(spec.cards.cardSpecifiers, *card))
                targets.push_back(card);
        }
    } else if (t.type == asn::TargetType::OppositeThis ||
               t.type == asn::TargetType::BattleOpponent) {
        auto card = mPlayer->oppositeCard(thisCard().card);
        if (!card)
            return targets;
        if (t.type == asn::TargetType::BattleOpponent) {
            const auto &spec = *t.targetSpecification;

            if (!checkTargetMode(spec.mode, thisCard().card, card))
                return targets;

            if (checkCard(spec.cards.cardSpecifiers, *card))
                targets.push_back(card);
        } else if (t.type == asn::TargetType::OppositeThis) {
            targets.push_back(card);
        }
    } else if (t.type == asn::TargetType::MentionedInTrigger) {
        if (mCardFromTrigger)
            targets.push_back(mCardFromTrigger);
    }
    return targets;
}

bool AbilityPlayer::findChooseTargetsAutomatically(const asn::ChooseCard &e) {
    if (e.targets.size() > 1)
        return false;
    assert(e.targets.size());

    const auto &target = e.targets[0].target;
    if (target.type == asn::TargetType::CharInBattle) {
        auto card = mPlayer->cardInBattle();
        if (!card)
            return false;

        addChosenCard(CardImprint(card->zone()->name(), card));
        return true;
    }

    return false;
}

bool AbilityPlayer::canBePayed(const asn::CostItem &c) {
    if (c.type == asn::CostType::Stock) {
        const auto &item = std::get<asn::StockCost>(c.costItem);
        if (item.value > mPlayer->zone("stock")->count())
            return false;
    } else {
        const auto &item = std::get<asn::Effect>(c.costItem);
        if (item.type == asn::EffectType::MoveCard) {
            const auto &e = std::get<asn::MoveCard>(item.effect);
            auto targets = getTargets(e.target, e.from.zone);
            if (targets.empty())
                return false;

            if (e.target.type == asn::TargetType::SpecificCards) {
                const auto &spec = *e.target.targetSpecification;
                if (spec.number.value > static_cast<int>(targets.size()))
                    return false;
            }
        } else if (item.type == asn::EffectType::ChangeState) {
            const auto &e = std::get<asn::ChangeState>(item.effect);
            if (e.target.type == asn::TargetType::ThisCard) {
                if (e.state == thisCard().card->state())
                    return false;
            } else if (e.target.type == asn::TargetType::SpecificCards) {
                int num = 0;
                auto targets = getTargets(e.target);
                for (auto target: targets) {
                    if (e.state != target->state())
                        num++;
                }
                const auto &spec = *e.target.targetSpecification;
                assert(spec.number.mod == asn::NumModifier::ExactMatch);
                if (num < spec.number.value)
                    return false;
            } else {
                assert(false);
            }
        } else if (item.type == asn::EffectType::RevealCard) {
            const auto &e = std::get<asn::RevealCard>(item.effect);
            assert(e.number.value == 1);
            auto hand = mPlayer->zone("hand");
            for (int i = 0; i < hand->count(); ++i)
                if (checkCard(e.card->cardSpecifiers, *hand->card(i)))
                    return true;

            return false;
        } else if (item.type == asn::EffectType::RemoveMarker) {
            const auto &e = std::get<asn::RemoveMarker>(item.effect);
            assert(e.targetMarker.type == asn::TargetType::SpecificCards);
            assert(e.markerBearer.type == asn::TargetType::ThisCard);
            const auto &spec = *e.targetMarker.targetSpecification;
            const auto &markers = thisCard().card->markers();
            if (markers.size() < spec.number.value)
                return false;

            return true;
        } else {
            assert(false);
        }
    }
    return true;
}

bool AbilityPlayer::canBePlayed(const asn::Ability &a) {
    if (a.type == asn::AbilityType::Auto) {
        const auto &aa = std::get<asn::AutoAbility>(a.ability);
        if (!aa.cost)
            return true;
        for (const auto &costItem: aa.cost->items)
            if (!canBePayed(costItem))
                return false;
        return true;
    } else if (a.type == asn::AbilityType::Act) {
        const auto &aa = std::get<asn::ActAbility>(a.ability);
        for (const auto &costItem: aa.cost.items)
            if (!canBePayed(costItem))
                return false;
    }

    return true;
}
