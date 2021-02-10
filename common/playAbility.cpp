#include "abilities.pb.h"

#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"


Resumable ServerPlayer::playNonMandatory(const asn::NonMandatory &e) {
    mContext.mandatory = false;
    for (const auto &effect: e.effect)
        co_await playEffect(effect);
    mContext.mandatory = true;
}

Resumable ServerPlayer::playChooseCard(const asn::ChooseCard &e) {
    std::vector<uint8_t> buf;
    encodeChooseCard(e, buf);

    EventChooseCard ev;
    ev.set_effect(buf.data(), buf.size());
    ev.set_mandatory(mContext.mandatory);
    sendToBoth(ev);

    clearExpectedComands();
    addExpectedCommand(CommandChooseCard::GetDescriptor()->name());
    addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            mContext.canceled = true;
            break;
        } else if (cmd.command().Is<CommandChooseCard>()) {
            CommandChooseCard chooseCmd;
            cmd.command().UnpackTo(&chooseCmd);
            // check more cases
            assert(e.targets.size() == 1);
            if (e.targets[0].type == asn::TargetType::SpecificCards) {
                auto &spec = e.targets[0].targetSpecification;
                if ((spec->number.mod == asn::NumModifier::ExactMatch &&
                    spec->number.value != chooseCmd.ids_size()) ||
                    (spec->number.mod == asn::NumModifier::AtLeast &&
                     spec->number.value < chooseCmd.ids_size()) ||
                    (spec->number.mod == asn::NumModifier::UpTo &&
                     spec->number.value > chooseCmd.ids_size()))
                continue;
            }
            //TODO: add checks
            for (int i = chooseCmd.ids_size() - 1; i >= 0; --i) {
                auto owner = chooseCmd.owner() == ProtoOwner::ProtoPlayer ? this : mGame->opponentOfPlayer(mId);
                auto pzone = owner->zone(chooseCmd.zone());
                if (!pzone)
                    break;

                auto card = pzone->card(chooseCmd.ids(i));
                if (!card)
                    break;
                mContext.chosenCards.push_back(CardImprint(chooseCmd.zone(), chooseCmd.ids(i), card, chooseCmd.owner() == ProtoOwner::ProtoOpponent));
            }
            break;
        }
    }
    clearExpectedComands();
}

Resumable ServerPlayer::playMoveCard(const asn::MoveCard &e) {
    assert(e.executor == asn::Player::Player);
    assert(e.to[0].zone != asn::Zone::Stage);

    if (e.target.type == asn::TargetType::ChosenCards) {
        int toIndex = 0;
        if (!mContext.chosenCards.size())
            co_return;

        if (e.to.size() > 1) {
            std::vector<uint8_t> buf;
            encodeMoveCard(e, buf);
            EventMoveChoice ev;
            ev.set_effect(buf.data(), buf.size());
            ev.set_mandatory(true);
            sendToBoth(ev);

            clearExpectedComands();
            addExpectedCommand(CommandChoice::GetDescriptor()->name());

            while (true) {
                auto cmd = co_await waitForCommand();
                if (cmd.command().Is<CommandChoice>()) {
                    CommandChoice choiceCmd;
                    cmd.command().UnpackTo(&choiceCmd);
                    toIndex = choiceCmd.choice();
                    if (static_cast<size_t>(toIndex) >= e.to.size())
                        continue;
                    break;
                }
            }
            clearExpectedComands();
        }
        for (const auto &card: mContext.chosenCards) {
            auto owner = card.opponent ? mGame->opponentOfPlayer(mId) : this;
            owner->moveCard(card.zone, card.id, asnZoneToString(e.to[toIndex].zone));
        }
    } else if (e.target.type == asn::TargetType::SpecificCards) {
        bool confirmed = mContext.mandatory;
        if (e.from.pos == asn::Position::Top) {
            if (zone(asnZoneToString(e.from.zone))->count() == 0)
                co_return;
        }
        assert(e.to.size() == 1);
        if (!mContext.mandatory) {
            std::vector<uint8_t> buf;
            encodeMoveCard(e, buf);

            EventMoveChoice ev;
            ev.set_effect(buf.data(), buf.size());
            ev.set_mandatory(false);
            sendToBoth(ev);

            clearExpectedComands();
            addExpectedCommand(CommandChoice::GetDescriptor()->name());

            while (true) {
                auto cmd = co_await waitForCommand();
                if (cmd.command().Is<CommandChoice>()) {
                    CommandChoice choiceCmd;
                    cmd.command().UnpackTo(&choiceCmd);
                    // 0 is yes, so 'yes' will be the first on client's side
                    confirmed = !choiceCmd.choice();
                    break;
                }
            }
            clearExpectedComands();
        }
        if (!confirmed)
            co_return;
        if (e.from.pos == asn::Position::Top) {
            if (e.from.zone == asn::Zone::Deck) {
                moveTopDeck(asnZoneToString(e.to[0].zone));
            } else {
                auto pzone = zone(asnZoneToString(e.from.zone));
                moveCard(asnZoneToString(e.from.zone), pzone->count() - 1, asnZoneToString(e.to[0].zone));
            }
        }
    } else if (e.target.type == asn::TargetType::ThisCard) {
        assert(mContext.mandatory);
        assert(e.to.size() == 1);
        moveCard(mContext.thisCard.zone, mContext.thisCard.id, asnZoneToString(e.to[0].zone));
    }
}

Resumable ServerPlayer::playDrawCard(const asn::DrawCard &e) {
    bool confirmed = mContext.mandatory;
    if (!mContext.mandatory) {
        std::vector<uint8_t> buf;
        encodeDrawCard(e, buf);

        EventDrawChoice ev;
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(false);
        sendToBoth(ev);

        clearExpectedComands();
        addExpectedCommand(CommandChoice::GetDescriptor()->name());

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                // 0 is yes, so 'yes' will be the first on client's side
                confirmed = !choiceCmd.choice();
                break;
            }
        }
        clearExpectedComands();
    }
    if (!confirmed)
        co_return;
    moveTopDeck("hand");
}

void ServerPlayer::playRevealCard(const asn::RevealCard &e) {
    if (e.type == asn::RevealType::TopDeck) {
        if (e.number.mod == asn::NumModifier::ExactMatch) {
            for (int i = 0; i < e.number.value; ++i) {
                auto deck = zone("deck");
                if (i >= deck->count())
                    break;

                EventRevealTopDeck event;
                event.set_code(deck->card(deck->count() - i - 1)->code());
                sendToBoth(event);
                mContext.mentionedCards.push_back(CardImprint("deck", deck->count() - i - 1, deck->card(deck->count() - i - 1)));
            }
        }
    }
}

void ServerPlayer::playAttributeGain(const asn::AttributeGain &e) {
    if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: mContext.chosenCards)
            addAttributeBuff(e.type, card.id, e.value, e.duration);
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
    }
    return false;
}

Resumable ServerPlayer::playEffect(const asn::Effect &e) {
    if (!evaluateCondition(e.cond)) {
        sendToBoth(EventConditionNotMet());
        co_return;
    }

    switch (e.type) {
    case asn::EffectType::NonMandatory:
        co_await playNonMandatory(std::get<asn::NonMandatory>(e.effect));
        break;
    case asn::EffectType::ChooseCard:
        co_await playChooseCard(std::get<asn::ChooseCard>(e.effect));
        break;
    case asn::EffectType::MoveCard:
        co_await playMoveCard(std::get<asn::MoveCard>(e.effect));
        break;
    case asn::EffectType::DrawCard:
        co_await playDrawCard(std::get<asn::DrawCard>(e.effect));
        break;
    case asn::EffectType::RevealCard:
        playRevealCard(std::get<asn::RevealCard>(e.effect));
        break;
    case asn::EffectType::AttributeGain:
        playAttributeGain(std::get<asn::AttributeGain>(e.effect));
        break;
    default:
        assert(false);
        break;
    }
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
        if (mQueue.size() > 1) {
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
        } else {
            uniqueId = mQueue[0].uniqueId;
        }

        j = 0;
        for (; j < mQueue.size(); ++j) {
            if (mQueue[j].uniqueId == uniqueId) {
                mContext = AbilityContext();
                mContext.thisCard = mQueue[j].card;
                // not a reliable way to get temporary ability (in case the card changed zone)
                auto &arr = mQueue[j].card.card->abilities();
                co_await playAbility(arr[mQueue[j].abilityId].ability);
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

