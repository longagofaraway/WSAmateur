#include <array>

#include "abilities.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"
#include "globalAbilities/globalAbilities.h"

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

void AbilityPlayer::removeMentionedCard(int cardPos) {
    std::erase_if(mMentionedCards, [cardPos](CardImprint &im) { return cardPos == im.id; });
}

Resumable ServerPlayer::resolveTrigger(ServerCard *card, asn::TriggerIcon trigger) {
    EventAbilityActivated event;
    auto ab = event.add_abilities();
    ab->set_zone("res");
    ab->set_type(ProtoAbilityType::ProtoClimaxTrigger);
    ab->set_cardid(0);
    ab->set_abilityid(static_cast<::google::protobuf::int32>(trigger));
    ab->set_cardcode(card->code());
    auto uniqueId = abilityHash(*ab);
    ab->set_uniqueid(uniqueId);
    sendToBoth(event);

    EventStartResolvingAbility evStart;
    evStart.set_uniqueid(uniqueId);
    sendToBoth(evStart);

    AbilityPlayer a(this);
    a.setThisCard(CardImprint("res", 0, card));
    co_await a.playAbility(triggerAbility(trigger));

    EventAbilityResolved ev2;
    ev2.set_uniqueid(uniqueId);
    sendToBoth(ev2);

    sendToBoth(EventEndResolvingAbilties());
}

bool ServerPlayer::canBePayed(ServerCard *thisCard, const asn::CostItem &c) {
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
        } else if (item.type == asn::EffectType::ChangeState) {
            const auto &e = std::get<asn::ChangeState>(item.effect);
            if (e.state == protoStateToState(thisCard->state()))
                return false;
        }
        return true;
    }
}

bool ServerPlayer::canBePlayed(ServerCard *thisCard, const asn::Ability &a) {
    if (a.type == asn::AbilityType::Auto) {
        const auto &aa = std::get<asn::AutoAbility>(a.ability);
        if (!aa.cost)
            return true;
        for (const auto &costItem: aa.cost->items)
            if (!canBePayed(thisCard, costItem))
                return false;
        return true;
    } else if (a.type == asn::AbilityType::Act) {
        const auto &aa = std::get<asn::ActAbility>(a.ability);
        for (const auto &costItem: aa.cost.items)
            if (!canBePayed(thisCard, costItem))
                return false;
    }

    return true;
}

void ServerPlayer::resolveAllContAbilities() {
    std::array<std::string_view, 3> zones{ "stage", "climax", "hand" };
    for (auto zoneName: zones) {
        auto pzone = zone(zoneName);
        for (int i = 0; i < pzone->count(); ++i) {
            auto card = pzone->card(i);
            if (!card)
                continue;

            playContAbilities(card);
        }
    }
}

namespace {
bool checkAbilityValidForZone(const asn::ContAbility &a, const std::string &name) {
    std::array<asn::EffectType, 2> e{ asn::EffectType::EarlyPlay,
                                      asn::EffectType::CannotPlay };
    return std::any_of(e.begin(), e.end(),[a](asn::EffectType type){ return a.effects[0].type == type; }) == (name == "hand");
}
}

void ServerPlayer::playContAbilities(ServerCard *card) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Cont)
            continue;
        const auto &cont = std::get<asn::ContAbility>(abs[i].ability.ability);

        if (!checkAbilityValidForZone(cont, card->zone()->name()))
            return;

        AbilityPlayer a(this);
        a.setThisCard(CardImprint(card->zone()->name(), card->pos(), card));
        a.setAbilityId(i);
        a.playContAbility(cont, abs[i].active);
    }
}

void ServerPlayer::deactivateContAbilities(ServerCard *source) {
    std::array<std::string_view, 2> zones{ "stage", "hand" };
    for (auto  zoneName: zones) {
        auto stage = zone(zoneName);
        for (int i = 0; i < stage->count(); ++i) {
            auto card = stage->card(i);
            if (!card)
                continue;

            auto oldAttrs = card->attributes();
            card->removeContBuffsBySource(source);
            sendChangedAttrs(card, oldAttrs);
        }
    }
}

void ServerPlayer::checkZoneChangeTrigger(ServerCard *movedCard, int cardId, std::string_view from, std::string_view to) {
    auto checkTrigger = [=](ServerCard *card, int id) {
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
            ta.card = CardImprint(card->zone()->name(), id, card);
            ta.type = ProtoCard;
            ta.abilityId = j;
            mQueue.push_back(ta);
        }
    };

    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;
        checkTrigger(card, i);
    }

    if (movedCard->zone()->name() != "stage")
        checkTrigger(movedCard, cardId);
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

void ServerPlayer::checkOnAttack(ServerCard *attCard) {
    auto &abs = attCard->abilities();
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
        a.card = CardImprint(attCard->zone()->name(), attCard->pos(), attCard);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

void ServerPlayer::checkPhaseTrigger(asn::PhaseState state, asn::Phase phase) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        auto &abs = card->abilities();
        for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
            if (abs[i].ability.type != asn::AbilityType::Auto)
                continue;
            const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
            if (autoab.trigger.type != asn::TriggerType::OnPhaseEvent)
                continue;
            const auto &trig = std::get<asn::PhaseTrigger>(autoab.trigger.trigger);
            if (trig.phase != phase || trig.state != state ||
                (trig.player == asn::Player::Player && !mActive) ||
                (trig.player == asn::Player::Opponent && mActive))
                continue;

            TriggeredAbility a;
            a.card = CardImprint(card->zone()->name(), card->pos(), card);
            a.type = ProtoCard;
            a.abilityId = i;
            mQueue.push_back(a);
        }
    }
}

void ServerPlayer::checkOnBackup(ServerCard *card) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnBackupOfThis)
            continue;

        TriggeredAbility a;
        a.card = CardImprint(card->zone()->name(), card->pos(), card);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

void ServerPlayer::checkOtherTrigger(const std::string &code) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        auto &abs = card->abilities();
        for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
            if (abs[i].ability.type != asn::AbilityType::Auto)
                continue;

            const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
            if (autoab.trigger.type != asn::TriggerType::OtherTrigger)
                continue;

            const auto &trig = std::get<asn::OtherTrigger>(autoab.trigger.trigger);
            if (code != trig.cardCode)
                continue;

            TriggeredAbility a;
            a.card = CardImprint(card->zone()->name(), card->pos(), card);
            a.type = ProtoCard;
            a.abilityId = i;
            mQueue.push_back(a);
        }
    }
}

void ServerPlayer::triggerBackupAbility(ServerCard *card) {
    if (!card->isCounter())
        return;

    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Act)
            continue;
        const auto &actab = std::get<asn::ActAbility>(abs[i].ability.ability);
        if (actab.keywords.empty() || actab.keywords[0] != asn::Keyword::Backup)
            continue;

        TriggeredAbility a;
        a.card = CardImprint(card->zone()->name(), card->pos(), card);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);

        return;
    }
}

void ServerPlayer::checkOnReversed(ServerCard *card) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnReversed)
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

Resumable ServerPlayer::playEventEffects(ServerCard *card) {
    auto expectedCopy = mExpectedCommands;
    clearExpectedComands();

    auto &abs = card->abilities();
    bool needEndEvent = false;
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Event)
            continue;

        needEndEvent = true;
        EventAbilityActivated event;
        auto eventAbility = event.add_abilities();
        eventAbility->set_zone(card->zone()->name());
        eventAbility->set_type(ProtoCard);
        eventAbility->set_cardid(card->pos());
        eventAbility->set_abilityid(i);
        eventAbility->set_cardcode(card->code());
        auto uniqueId = abilityHash(*eventAbility);
        eventAbility->set_uniqueid(uniqueId);
        sendToBoth(event);

        EventStartResolvingAbility evStart;
        evStart.set_uniqueid(uniqueId);
        sendToBoth(evStart);

        AbilityPlayer a(this);
        a.setThisCard(CardImprint(card->zone()->name(), card->pos(), card));
        co_await a.playAbility(abs[i].ability);

        EventAbilityResolved evEnd;
        evEnd.set_uniqueid(uniqueId);
        sendToBoth(evEnd);
    }

    if (needEndEvent)
        sendToBoth(EventEndResolvingAbilties());

    mExpectedCommands = expectedCopy;
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
    // 6. After all abilities are resolved, server sends EventEndResolvingAbilties.
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
            if (canBePlayed(mQueue[i].card.card, mQueue[i].getAbility())) {
                uniqueId = mQueue[i].uniqueId;
                playableCount++;
            }
        }

        if (playableCount > 1) {
            clearExpectedComands();
            addExpectedCommand(CommandPlayAbility::descriptor()->name());
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

                AbilityPlayer a(this);
                a.setThisCard(mQueue[j].card);
                // not a reliable way to get temporary ability (in case the card changed zone)
                co_await a.playAbility(mQueue[j].getAbility());

                EventAbilityResolved evEnd;
                evEnd.set_uniqueid(uniqueId);
                sendToBoth(evEnd);
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
            AbilityPlayer a(this);
            co_await a.playAbility(ruleActionAbility(static_cast<RuleAction>(it->abilityId)));
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

