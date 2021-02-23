#include "abilities.pb.h"

#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"

Resumable ServerPlayer::payCost() {
    if (!mContext.cost)
        co_return;

    for (const auto &item: mContext.cost->items) {
        if (item.type == asn::CostType::Stock) {
            for (int i = 0; i < std::get<asn::StockCost>(item.costItem).value; ++i)
                moveCard("stock", zone("stock")->count() - 1, "wr");
        } else {
            co_await playEffect(std::get<asn::Effect>(item.costItem));
        }
    }
}

bool ServerPlayer::evaluateCondition(const asn::Condition &c) {
    switch (c.type) {
    case asn::ConditionType::NoCondition:
        return true;
    case asn::ConditionType::IsCard:
        return evaluateConditionIsCard(std::get<asn::ConditionIsCard>(c.cond));
    default:
        assert(false);
        return false;
    }
}

bool ServerPlayer::evaluateConditionIsCard(const asn::ConditionIsCard &c) {
    if (c.target.type == asn::TargetType::MentionedCards) {
        for (const auto &card: mContext.mentionedCards) {
            assert(card.card);
            for (const auto &neededCard: c.neededCard)
                if (checkCard(neededCard.cardSpecifiers, *card.card))
                    return true;
        }
    } else if (c.target.type == asn::TargetType::SpecificCards) {
        assert(c.neededCard.size() == 1);
        const auto &spec = *c.target.targetSpecification;
        if (spec.mode == asn::TargetMode::All) {
            auto stage = zone("stage");
            bool verified = true;
            for (int i = 0; i < stage->count(); ++i)
                if (stage->card(i) && !checkCard(c.neededCard[0].cardSpecifiers, *stage->card(i)))
                        verified = false;
            if (verified)
                return true;
        }
    }
    return false;
}

Resumable ServerPlayer::playEventAbility(const asn::EventAbility &a) {
    for (const auto &effect: a.effects)
        co_await playEffect(effect);
}

Resumable ServerPlayer::playAutoAbility(const asn::AutoAbility &a) {
    mContext.cost = a.cost;
    for (const auto &effect: a.effects)
        co_await playEffect(effect);
}

void ServerPlayer::playContAbility(const asn::ContAbility &a, bool &active) {
    if (a.effects.size() == 0)
        return;

    bool res = evaluateCondition(a.effects[0].cond);
    if ((res && active) ||
        (!res && !active))
        return;
    if (!res && active) {
        mContext.revert = true;
        active = false;
    } else {
        active = true;
    }
    for (const auto &effect: a.effects)
        playContEffect(effect);
}

Resumable ServerPlayer::playAbility(const asn::Ability &a) {
    switch(a.type) {
    case asn::AbilityType::Auto:
        co_await playAutoAbility(std::get<asn::AutoAbility>(a.ability));
        break;
    case asn::AbilityType::Event:
        co_await playEventAbility(std::get<asn::EventAbility>(a.ability));
        break;
    default:
        break;
    }
}

void ServerPlayer::activateContAbilities(ServerCard *card) {
    for (auto &ab: card->abilities()) {
        if (ab.ability.type != asn::AbilityType::Cont)
            continue;

        mContext = AbilityContext();
        mContext.thisCard = CardImprint(card->zone()->name(), card->pos(), card);
        mContext.cont = true;
        playContAbility(std::get<asn::ContAbility>(ab.ability.ability), ab.active);
    }
}

bool ServerPlayer::canBePayed(const asn::CostItem &c) {
    if (c.type == asn::CostType::Stock) {
        const auto &item = std::get<asn::StockCost>(c.costItem);
        if (item.value > zone("stock")->count())
            return false;
        return true;
    } else {
        const auto &item = std::get<asn::Effect>(c.costItem);
        if (item.type == asn::EffectType::MoveCard) {
            const auto &e = std::get<asn::MoveCard>(item.effect);
            if (e.from.zone == asn::Zone::Hand &&
                e.target.targetSpecification->number.value > zone("hand")->count())
                return false;
        }
        return true;
    }
}

bool ServerPlayer::canBePlayed(const asn::Ability &a) {
    if (a.type == asn::AbilityType::Auto) {
        const auto &aa = std::get<asn::AutoAbility>(a.ability);
        if (!aa.cost)
            return true;
        for (const auto &costItem: aa.cost->items)
            if (!canBePayed(costItem))
                return false;
        return true;
    }

    return false;
}

void ServerPlayer::stageCountChanged() {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        for (auto &ab: card->abilities()) {
            if (ab.ability.type != asn::AbilityType::Cont)
                continue;
            const auto &cont = std::get<asn::ContAbility>(ab.ability.ability);
            if (cont.effects[0].cond.type == asn::ConditionType::IsCard) {
                mContext = AbilityContext();
                mContext.thisCard = CardImprint(card->zone()->name(), card->pos(), card);
                mContext.cont = true;
                playContAbility(cont, ab.active);
            }
        }
    }
}

void ServerPlayer::checkOnPlacedFromHandToStage(ServerCard *card) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnZoneChange)
            continue;
        const auto &trig = std::get<asn::ZoneChangeTrigger>(autoab.trigger.trigger);
        assert(trig.target.size() == 1);
        if (trig.target[0].type != asn::TargetType::ThisCard)
            continue;
        if (trig.from != asn::Zone::Hand || trig.to != asn::Zone::Stage)
            continue;
        TriggeredAbility a;
        a.card = CardImprint(card->zone()->name(), card->pos(), card);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

Resumable ServerPlayer::checkTiming() {
    auto expectedCopy = mExpectedCommands;
    clearExpectedComands();
    // first play all rule actions
    // then if only 1 ability in queue, play it
    // otherwise wait for player to choose the ability
    bool abilitiesSent = false;
    while (mQueue.size()) {
        bool ruleActionFound = false;
        size_t j = 0;
        for (; j < mQueue.size(); ++j) {
            if (mQueue[j].type == ProtoAbilityType::ProtoRuleAction) {
                //play rule action
                //playAbility()
                ruleActionFound = true;
                break;
            }
        }
        if (ruleActionFound) {
            mQueue.erase(mQueue.begin() + j);
            continue;
        }

        if (!abilitiesSent) {
            EventAbilityActivated event;
            for (size_t i = 0; i < mQueue.size(); ++i) {
                auto ab = event.add_abilities();
                ab->set_zone(mQueue[i].card.zone);
                ab->set_type(mQueue[i].type);
                ab->set_cardid(mQueue[i].card.id);
                ab->set_abilityid(mQueue[i].abilityId);
                ab->set_cardcode(mQueue[i].card.card->code());
                mQueue[i].uniqueId = abilityHash(*ab);
                ab->set_uniqueid(mQueue[i].uniqueId);
            }
            sendToBoth(event);
            abilitiesSent = true;
        }

        uint32_t uniqueId;
        int playableCount = 0;
        for (size_t i = 0; i < mQueue.size(); ++i) {
            if (canBePlayed(mQueue[i].getAbility())) {
                uniqueId = mQueue[i].uniqueId;
                playableCount++;
            }
        }

        if (playableCount > 1) {
            clearExpectedComands();
            addExpectedCommand(CommandPlayAbility::GetDescriptor()->name());
            while (true) {
                auto cmd = co_await waitForCommand();
                if (cmd.command().Is<CommandPlayAbility>()) {
                    CommandPlayAbility choiceCmd;
                    cmd.command().UnpackTo(&choiceCmd);
                    uniqueId = choiceCmd.uniqueid();
                    break;
                }
            }
        } else if (playableCount == 0) {
            // Only abilities that cannot be played or cannot be paid for are left
            while(mQueue.size()) {
                EventAbilityResolved ev2;
                ev2.set_uniqueid(mQueue[0].uniqueId);
                sendToBoth(ev2);
                mQueue.erase(mQueue.begin());
            }
            continue;
        }

        j = 0;
        for (; j < mQueue.size(); ++j) {
            if (mQueue[j].uniqueId == uniqueId) {
                mContext = AbilityContext();
                mContext.thisCard = mQueue[j].card;
                // not a reliable way to get temporary ability (in case the card changed zone)
                co_await playAbility(mQueue[j].getAbility());
                break;
            }
        }
        EventAbilityResolved ev2;
        ev2.set_uniqueid(uniqueId);
        sendToBoth(ev2);
        mQueue.erase(mQueue.begin() + j);
    }
    mExpectedCommands = expectedCopy;
}

