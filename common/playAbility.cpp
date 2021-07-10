#include <array>

#include "abilities.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"
#include "globalAbilities/globalAbilities.h"

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
            assert(e.target.type == asn::TargetType::SpecificCards);
            if (e.from.zone == asn::Zone::Hand && e.target.type == asn::TargetType::SpecificCards &&
                e.target.targetSpecification->number.value > zone("hand")->count())
                return false;
        } else if (item.type == asn::EffectType::ChangeState) {
            const auto &e = std::get<asn::ChangeState>(item.effect);
            assert(e.target.type == asn::TargetType::ThisCard);
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

void ServerPlayer::playContAbilities(ServerCard *card, bool revert) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Cont)
            continue;
        const auto &cont = std::get<asn::ContAbility>(abs[i].ability.ability);

        if (!checkAbilityValidForZone(cont, card->zone()->name()))
            continue;

        if (revert && !abs[i].active)
            continue;

        AbilityPlayer a(this);
        a.setThisCard(CardImprint(card->zone()->name(), card));
        a.setAbilityId(abs[i].id);
        if (!revert) {
            a.playContAbility(cont, abs[i].active);
        } else {
            a.revertContAbility(cont);
            abs[i].active = false;
        }
    }
}

void ServerPlayer::deactivateContAbilities(ServerCard *source) {
    std::array<std::string_view, 2> zones{ "stage", "hand" };
    for (auto zoneName: zones) {
        auto pzone = zone(zoneName);
        for (int i = 0; i < pzone->count(); ++i) {
            auto card = pzone->card(i);
            if (!card)
                continue;

            auto oldAttrs = card->attributes();
            card->removeContBuffsBySource(source);
            sendChangedAttrs(card, oldAttrs);
        }
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
        eventAbility->set_cardid(card->id());
        eventAbility->set_abilityid(i);
        eventAbility->set_cardcode(card->code());
        auto uniqueId = abilityHash(*eventAbility);
        eventAbility->set_uniqueid(uniqueId);
        sendToBoth(event);

        EventStartResolvingAbility evStart;
        evStart.set_uniqueid(uniqueId);
        sendToBoth(evStart);

        AbilityPlayer a(this);
        a.setThisCard(CardImprint(card->zone()->name(), card));
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
            ab->set_cardid(mQueue[i].card.card->id());
            ab->set_abilityid(mQueue[i].abilityId);
            ab->set_cardcode(mQueue[i].card.card->code());
            if (mQueue[i].ability) {
                auto enc = encodeAbility(*mQueue[i].ability);
                ab->set_ability(enc.data(), enc.size());
            }
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

