#include "abilities.pb.h"

#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"
#include "globalAbilities/globalAbilities.h"

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
    case asn::ConditionType::HaveCards:
        return evaluateConditionHaveCard(std::get<asn::ConditionHaveCard>(c.cond));
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

bool ServerPlayer::evaluateConditionHaveCard(const asn::ConditionHaveCard &c) {
    auto player = (c.who == asn::Player::Player) ? this : mGame->opponentOfPlayer(mId);
    auto z = player->zone(asnZoneToString(c.where.zone));
    int count = 0;
    for (int i = 0; i < z->count(); ++i) {
        auto card = z->card(i);
        if (!card)
            continue;

        if (c.excludingThis && mContext.thisCard.card == card)
            continue;

        if (checkCard(c.whichCards.cardSpecifiers, *card)) {
            count++;

            if (c.howMany.mod == asn::NumModifier::AtLeast &&
                count >= c.howMany.value)
                return true;
        }
    }
    if ((c.howMany.mod == asn::NumModifier::ExactMatch &&
         c.howMany.value == count) ||
        (c.howMany.mod == asn::NumModifier::UpTo &&
         c.howMany.value <= count))
        return true;

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

void ServerPlayer::validateContAbilitiesOnStageChanges() {
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

void ServerPlayer::checkZoneChangeTrigger(ServerCard *movedCard, std::string_view from, std::string_view to) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;
        for (int j = 0; static_cast<size_t>(j) < card->abilities().size(); ++j) {
            const auto &a = card->abilities()[j];
            if (a.ability.type != asn::AbilityType::Auto)
                continue;
            const auto &aa = std::get<asn::AutoAbility>(a.ability.ability);
            if (aa.trigger.type != asn::TriggerType::OnZoneChange)
                continue;
            const auto &t = std::get<asn::ZoneChangeTrigger>(aa.trigger.trigger);
            if (t.from != asn::Zone::NotSpecified && asnZoneToString(t.from) != from)
                continue;
            if (t.to != asn::Zone::NotSpecified && asnZoneToString(t.to) != to)
                continue;
            assert(t.target.size() == 1);
            if (t.target[0].type == asn::TargetType::ThisCard) {
                if (card != movedCard)
                    continue;
            } else if (t.target[0].type != asn::TargetType::SpecificCards) {
                const auto &spec = *t.target[0].targetSpecification;
                if (!checkCard(spec.cards.cardSpecifiers, *movedCard))
                    continue;
            }

            TriggeredAbility ta;
            ta.card = CardImprint(card->zone()->name(), card->pos(), card);
            ta.type = ProtoCard;
            ta.abilityId = j;
            mQueue.push_back(ta);
        }
    }

}

void ServerPlayer::checkGlobalEncore(ServerCard *movedCard, int cardId, std::string_view from, std::string_view to) {
    if (from == "stage" && to == "wr" && movedCard->zone()->name() == "wr") {
        TriggeredAbility ta;
        ta.card = CardImprint(movedCard->zone()->name(), cardId, movedCard);
        ta.type = ProtoGlobal;
        ta.abilityId = static_cast<int>(GlobalAbility::Encore);
        mQueue.push_back(ta);
    }
}

void ServerPlayer::checkOnAttack(ServerCard *card) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnAttack)
            continue;
        const auto &trig = std::get<asn::OnAttackTrigger>(autoab.trigger.trigger);
        if (trig.target.type != asn::TargetType::ThisCard)
            continue;
        TriggeredAbility a;
        a.card = CardImprint(card->zone()->name(), card->pos(), card);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

void ServerPlayer::checkOnBattleOpponentReversed(ServerCard *attCard, ServerCard *battleOpp) {
    auto &abs = attCard->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnBattleOpponentReversed)
            continue;
        const auto &trig = std::get<asn::BattleOpponentReversedTrigger>(autoab.trigger.trigger);
        if (!checkCard(trig.card.cardSpecifiers, *battleOpp))
            continue;
        TriggeredAbility a;
        a.card = CardImprint(attCard->zone()->name(), attCard->pos(), attCard);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

Resumable ServerPlayer::checkTiming() {
    // protocol for playing abilities
    // 1. Server sends EventAbilityActivated with activated abilities. Client shows them.
    // 2. If there are more than 1 playable ability, client must send, which one he wants to play
    //    using CommandPlayAbility. Otherwise, server will automatically choose to play the single playable ability.
    // 3. Server sends EventStartResolvingAbility to indicate, which ability it is going to play.
    //    Client marks that ability as active.
    // 4. Server sends EventAbilityResolved after the ability is resolved.
    // 5. At this point more abilities may be triggered. If this is not the master of the turn and
    //    the master of the turn has abilities to resolve, send EventEndResolvingAbilties as this player,
    //    and perform checkTiming as active player. If new abilities appeared during check timing,
    //    proceed to point 1 but send EventAbilityActivated only with new abilities. If there are no new abilities,
    //    but not all abilities are resolved, proceed to point 2.
    // 6. After all abilities are resolved server sends EventEndResolvingAbilties.
    //    Client uses this signal to restore ui state after playing abilities.
    if (mQueue.empty())
        co_return;

    auto expectedCopy = mExpectedCommands;
    clearExpectedComands();
    while (mQueue.size()) {
        // first play all rule actions
        bool ruleActionFound = false;
        co_await processRuleActions(ruleActionFound);
        if (ruleActionFound)
            continue;
        co_await mGame->opponentOfPlayer(mId)->processRuleActions(ruleActionFound);

        if (!mActive && mGame->opponentOfPlayer(mId)->hasActivatedAbilities()) {
            // priority to the master of the turn
            for (size_t i = 0; i < mQueue.size(); ++i)
                mQueue[i].uniqueId = 0;
            break;
        }

        EventAbilityActivated event;
        for (size_t i = 0; i < mQueue.size(); ++i) {
            if (mQueue[i].uniqueId != 0)
                continue;
            auto ab = event.add_abilities();
            ab->set_zone(mQueue[i].card.zone);
            ab->set_type(mQueue[i].type);
            ab->set_cardid(mQueue[i].card.id);
            ab->set_abilityid(mQueue[i].abilityId);
            ab->set_cardcode(mQueue[i].card.card->code());
            mQueue[i].uniqueId = abilityHash(*ab);
            ab->set_uniqueid(mQueue[i].uniqueId);
        }
        if (event.abilities_size())
            sendToBoth(event);

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

        for (size_t j = 0; j < mQueue.size(); ++j) {
            if (mQueue[j].uniqueId == uniqueId) {
                EventStartResolvingAbility evStart;
                evStart.set_uniqueid(uniqueId);
                sendToBoth(evStart);

                mContext = AbilityContext();
                mContext.thisCard = mQueue[j].card;
                // not a reliable way to get temporary ability (in case the card changed zone)
                co_await playAbility(mQueue[j].getAbility());

                EventAbilityResolved ev2;
                ev2.set_uniqueid(uniqueId);
                sendToBoth(ev2);
                mQueue.erase(mQueue.begin() + j);

                break;
            }
        }
    }
    sendToBoth(EventEndResolvingAbilties());

    mExpectedCommands = expectedCopy;
}

Resumable ServerPlayer::processRuleActions(bool &ruleActionFound) {
    if (mQueue.empty())
        co_return;

    auto it = mQueue.begin();
    while (it != mQueue.end()) {
        if (it->type == ProtoAbilityType::ProtoRuleAction) {
            mContext = AbilityContext();
            co_await playAbility(ruleActionAbility(static_cast<RuleAction>(it->abilityId)));
            ruleActionFound = true;
            it = mQueue.erase(it);
        } else {
            ++it;
        }
    }
}

bool ServerPlayer::hasActivatedAbilities() const {
    return !mQueue.empty();
}

