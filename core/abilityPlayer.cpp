#include "abilityPlayer.h"

#include <QDebug>

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"

#include "abilityUtils.h"
#include "serverPlayer.h"
#include "serverGame.h"

namespace {
int numOfDifferentlyNamedEvents(ServerPlayer *player) {
    auto zone = player->zone(asn::Zone::Memory);
    std::set<std::string> names;
    for (int i = 0; i < zone->count(); ++i) {
        auto card = zone->card(i);
        names.insert(card->name());
    }
    return static_cast<int>(names.size());
}

bool hasHinaCondition(const asn::ChooseCard &e) {
    for (const auto &target: e.targets) {
        if (target.target.type != asn::TargetType::SpecificCards) {
            continue;
        }
        const auto &spec = *target.target.targetSpecification;
        for (const auto& cardSpecifier: spec.cards.cardSpecifiers) {
            if (cardSpecifier.type ==
                    asn::CardSpecifierType::SumOfLevelsLessThanDiffNamedEventsInMemory) {
                return true;
            }
        }
    }
    return false;
}

}

void AbilityPlayer::setContext(const AbilityContext &context) {
    for (const auto &cardImprint: context.chosenCards) {
        if (cardImprint.card && cardImprint.card->zone()->name() == cardImprint.zone) {
            mChosenCards.push_back(cardImprint);
        }
    }
    for (const auto &cardImprint: context.lastMovedCards) {
        if (cardImprint.card && cardImprint.card->zone()->name() == cardImprint.zone) {
            mLastMovedCards.push_back(cardImprint);
        }
    }
}

Resumable AbilityPlayer::payCost() {
    if (!hasCost())
        co_return;

    mPlayer->game()->triggerManager()->payingCostEvent(mThisCard.card, mAbilityType);
    co_await playTriggeredAbilities(mPlayer->helperQueue());

    setPayingCost(true);
    for (const auto &item: cost().items) {
        if (item.type == asn::CostType::Stock) {
            int cost = std::get<asn::StockCost>(item.costItem).value - mPlayer->costReduction();
            if (cost < 0) cost = 0;
            mPlayer->resetCostReduction();
            for (int i = 0; i < cost; ++i)
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
    mPlayer->game()->checkOnActAbility(mPlayer->id());
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

int AbilityPlayer::timesCanBePerformed(const asn::Effect &effect) {
    if (effect.type == asn::EffectType::RemoveMarker) {
        const auto &markerEffect = std::get<asn::RemoveMarker>(effect.effect);
        assert(markerEffect.markerBearer.type == asn::TargetType::ThisCard);
        if (mThisCard.zone != mThisCard.card->zone()->name())
            return 0;
        int numberOfTimes = 0;
        for (const auto &marker: thisCard().card->markers()) {
            if (!checkCardMatches(marker.get(), markerEffect.targetMarker, thisCard().card))
                continue;
            numberOfTimes++;
        }
        // assume 1 each time
        return numberOfTimes;
    } else {
        qWarning() << "effect" << static_cast<int>(effect.type) << "in timesCanBePerformed is not supported";
    }
    return 0;
}

Resumable AbilityPlayer::playAbility(const asn::Ability a) {
    setCont(false);
    mAbilityType = a.type;
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

Resumable AbilityPlayer::playTriggeredAbilities(const std::vector<TriggeredAbility> &abilities) {
    for (const auto &ability: abilities) {
        AbilityPlayer abilityPlayer(mPlayer);
        abilityPlayer.setThisCard(ability.card.card);
        abilityPlayer.setCardFromTrigger(ability.cardFromTrigger);
        co_await abilityPlayer.playAbility(ability.getAbility());
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
    std::erase_if(mMentionedCards, [cardId](CardImprint &im) { return im.card && cardId == im.card->id(); });
}

void AbilityPlayer::removeMentionedCard(ServerCard* card) {
    if (mMentionedCards.empty())
        return;

    if (mMentionedCards.front().card && mMentionedCards.front().card->player() != card->player())
        return;

    removeMentionedCard(card->id());
}

bool AbilityPlayer::isCardRevealed(ServerCard* card) {
    if (!mIsRevealing)
        return false;

    for (const auto& cardImprint: mMentionedCards) {
        if (cardImprint.card && cardImprint.card->player() == card->player() && cardImprint.card->id() == card->id()) {
            return true;
        }
    }
    return false;
}

std::vector<ServerCard*> AbilityPlayer::getTargets(
        const asn::Target &t,
        asn::PlaceType placeType,
        std::optional<asn::Place> place
) {
    if (placeType == asn::PlaceType::SpecificPlace && !place.has_value()) {
        place = asn::Place{
            asn::Position::NotSpecified,
            asn::Zone::Stage,
            asn::Player::Player
        };
    }
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
        if (placeType == asn::PlaceType::Selection) {
            for (const auto &card: mentionedCards()) {
                if (checkTarget(spec, card.card))
                    targets.push_back(card.card);
            }
        } else if (placeType == asn::PlaceType::SpecificPlace) {
            assert(place->owner != asn::Player::Both);
            auto player = owner(place->owner);
            auto pzone = player->zone(place->zone);
            for (int i = 0; i < pzone->count(); ++i) {
                auto card = pzone->card(i);
                if (checkTarget(spec, card))
                    targets.push_back(card);
            }
        }
    } else if (t.type == asn::TargetType::OppositeThis ||
               t.type == asn::TargetType::BattleOpponent) {
        auto card = mPlayer->oppositeCard(thisCard().card);
        if (!card)
            return targets;
        if (t.type == asn::TargetType::BattleOpponent) {
            const auto &spec = *t.targetSpecification;

            if (checkTarget(spec, card))
                targets.push_back(card);
        } else if (t.type == asn::TargetType::OppositeThis) {
            targets.push_back(card);
        }
    } else if (t.type == asn::TargetType::MentionedInTrigger) {
        if (mCardFromTrigger)
            targets.push_back(mCardFromTrigger);
    } else if (t.type == asn::TargetType::AttackingChar) {
        if (mPlayer->attackingCard())
            targets.push_back(mPlayer->attackingCard());
    }
    return targets;
}

std::vector<ServerCard *> AbilityPlayer::getTargetsFromAllZones(const asn::Target &t) {
    std::vector<ServerCard*> targets;
    if (t.type == asn::TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;

        const auto &zones = mPlayer->zones();
        for (const auto &zoneIt: zones) {
            const auto &zone = zoneIt.second;
            for (int i = 0; i < zone->count(); ++i) {
                auto card = zone->card(i);
                if (checkTarget(spec, card))
                    targets.push_back(card);
            }
        }
    } else {
        return getTargets(t);
    }
    return targets;
}

bool AbilityPlayer::checkTarget(const asn::TargetSpecificCards &spec, ServerCard *card) {
    if (!card)
        return false;

    if (!checkTargetMode(spec.mode, thisCard().card, card))
        return false;

    return checkCard(spec.cards.cardSpecifiers, *card, this);
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
            auto targets = getTargets(e.target, asn::PlaceType::SpecificPlace, e.from);
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
            assert(e.type == asn::RevealType::FromHand);
            auto hand = mPlayer->zone("hand");
            for (int i = 0; i < hand->count(); ++i)
                if (checkCard(e.card->cardSpecifiers, *hand->card(i)))
                    return true;

            return false;
        } else if (item.type == asn::EffectType::AddMarker) {
            const auto &e = std::get<asn::AddMarker>(item.effect);
            auto targets = getTargets(e.target, asn::PlaceType::SpecificPlace, e.from);
            if (targets.empty())
                return false;

            if (e.target.type == asn::TargetType::SpecificCards) {
                const auto &spec = *e.target.targetSpecification;
                if (spec.number.value > static_cast<int>(targets.size()))
                    return false;
            }
        } else if (item.type == asn::EffectType::RemoveMarker) {
            const auto &e = std::get<asn::RemoveMarker>(item.effect);
            assert(e.targetMarker.type == asn::TargetType::SpecificCards);
            assert(e.markerBearer.type == asn::TargetType::ThisCard);
            const auto &spec = *e.targetMarker.targetSpecification;
            const auto &markers = thisCard().card->markers();
            if (markers.size() < spec.number.value)
                return false;

            return true;
        } else if (item.type == asn::EffectType::Shuffle) {
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

Resumable AbilityPlayer::waitForPlayerLookConfirmation() {
    if (mentionedCards().empty())
        co_return;

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandPlayEffect::descriptor()->name());

    mPlayer->sendToBoth(EventConfirmationRequest());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandPlayEffect>())
            break;
    }
}

bool AbilityPlayer::checkHinaCondition(const asn::ChooseCard &e, const CommandChooseCard& cmd, ServerCardZone* zone) {
    if (!hasHinaCondition(e)) {
        return true;
    }

    int sumOfLevels{0};
    for (int i = cmd.positions_size() - 1; i >= 0; --i) {
        auto card = zone->card(cmd.positions(i));
        if (!card)
            break;
        sumOfLevels += card->level();
    }
    return sumOfLevels <= numOfDifferentlyNamedEvents(mPlayer);
}

bool AbilityPlayer::areChosenCardsValid(const asn::ChooseCard &e, const CommandChooseCard& cmd, ServerCardZone* zone) {
    return checkHinaCondition(e, cmd, zone);
}
