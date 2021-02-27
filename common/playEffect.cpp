#include "abilities.pb.h"

#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"

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
    case asn::EffectType::PayCost:
        co_await playPayCost(std::get<asn::PayCost>(e.effect));
        break;
    case asn::EffectType::SearchCard:
        co_await playSearchCard(std::get<asn::SearchCard>(e.effect));
        break;
    case asn::EffectType::Shuffle:
        playShuffle(std::get<asn::Shuffle>(e.effect));
        break;
    case asn::EffectType::AbilityGain:
        co_await playAbilityGain(std::get<asn::AbilityGain>(e.effect));
        break;
    default:
        assert(false);
        break;
    }
}

void ServerPlayer::playContEffect(const asn::Effect &e) {
    switch (e.type) {
    case asn::EffectType::AttributeGain:
        playAttributeGain(std::get<asn::AttributeGain>(e.effect));
        break;
    default:
        break;
    }
}

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
    // TODO: check for legitimacy of cancel
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

    if (e.target.type == asn::TargetType::ChosenCards) {
        int toIndex = 0;
        if (!mContext.chosenCards.size())
            co_return;

        // choice of a destination
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
            owner->moveCard(card.zone, card.id, asnZoneToString(e.to[toIndex].zone), mContext.revealChosen);
            if (zone("deck")->count() == 0)
                refresh();
            if (e.to[toIndex].zone == asn::Zone::Clock && zone("clock")->count() >= 7)
                co_await levelUp();
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
        // TODO: refresh and levelup are triggered at the same time, give choice
        if (e.to[0].zone == asn::Zone::Clock && zone("clock")->count() >= 7)
            co_await levelUp();
    } else if (e.target.type == asn::TargetType::ThisCard) {
        assert(mContext.mandatory);
        assert(e.to.size() == 1);
        if (mContext.thisCard.zone != mContext.thisCard.card->zone()->name())
            co_return;
        int toIndex = 0;
        if (e.to[0].pos == asn::Position::SlotThisWasIn)
            toIndex = mContext.thisCard.card->pos();
        moveCard(mContext.thisCard.zone, mContext.thisCard.id, asnZoneToString(e.to[0].zone), toIndex);
        if (e.to[0].pos == asn::Position::SlotThisWasIn)
            setCardState(mContext.thisCard.card, CardState::StateRested);
        if (e.to[0].zone == asn::Zone::Clock && zone("clock")->count() >= 7)
            co_await levelUp();
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
    switch (e.type) {
    case asn::RevealType::TopDeck:
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
        } else {
            assert(false);
        }
        break;
    case asn::RevealType::ChosenCards:
        mContext.revealChosen = true;
        break;
    default:
        assert(false);
        break;
    }
}

void ServerPlayer::playAttributeGain(const asn::AttributeGain &e) {
    if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: mContext.chosenCards)
            addAttributeBuff(e.type, card.id, e.value, e.duration);
    } else if (e.target.type == asn::TargetType::ThisCard) {
        auto card = mContext.thisCard.card;
        if (card == nullptr || card->zone()->name() != "stage")
            return;

        if (mContext.cont)
            changeAttribute(card, e.type, e.value * (mContext.revert ? -1 : 1));
        else
            addAttributeBuff(e.type, mContext.thisCard.id, e.value, e.duration);
    }
}

Resumable ServerPlayer::playPayCost(const asn::PayCost &e) {
    sendToBoth(EventPayCost());

    clearExpectedComands();
    addExpectedCommand(CommandPlayEffect::GetDescriptor()->name());
    addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            mContext.canceled = true;
            break;
        } else if (cmd.command().Is<CommandPlayEffect>()) {
            co_await payCost();
            break;
        }
    }

    if (!mContext.canceled)
        for (const auto &effect: e.ifYouDo)
            co_await playEffect(effect);

    if (mContext.canceled)
        for (const auto &effect: e.ifYouDont)
            co_await playEffect(effect);
    co_return;
}


Resumable ServerPlayer::playSearchCard(const asn::SearchCard &e) {
    std::vector<uint8_t> buf;
    encodeSearchCard(e, buf);

    EventSearchCard eventPrivate;
    eventPrivate.set_effect(buf.data(), buf.size());
    EventSearchCard eventPublic(eventPrivate);
    auto pzone = zone(asnZoneToString(e.place.zone));
    for (int i = 0; i < pzone->count(); ++i)
        eventPrivate.add_codes(pzone->card(i)->code());
    for (int i = 0; i < pzone->count(); ++i)
        eventPublic.add_codes("");
    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    clearExpectedComands();
    addExpectedCommand(CommandChooseCard::GetDescriptor()->name());
    // TODO: check for legitimacy of cancel
    addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            break;
        } else if (cmd.command().Is<CommandChooseCard>()) {
            CommandChooseCard chooseCmd;
            cmd.command().UnpackTo(&chooseCmd);

            assert(e.targets.size() == 1);
            if ((e.targets[0].number.mod == asn::NumModifier::ExactMatch &&
                e.targets[0].number.value != chooseCmd.ids_size()) ||
                (e.targets[0].number.mod == asn::NumModifier::AtLeast &&
                e.targets[0].number.value < chooseCmd.ids_size()) ||
                (e.targets[0].number.mod == asn::NumModifier::UpTo &&
                e.targets[0].number.value > chooseCmd.ids_size()))
                continue;
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

void ServerPlayer::playShuffle(const asn::Shuffle &e) {
    assert(e.owner != asn::Player::Both);
    assert(e.owner != asn::Player::NotSpecified);
    auto owner = (e.owner == asn::Player::Player) ? this : mGame->opponentOfPlayer(mId);
    owner->zone(asnZoneToString(e.zone))->shuffle();
}

Resumable ServerPlayer::playAbilityGain(const asn::AbilityGain &e) {
    if (static_cast<size_t>(e.number) < e.abilities.size()) {
        std::vector<uint8_t> buf;
        encodeAbilityGain(e, buf);

        EventAbilityChoice event;
        event.set_effect(buf.data(), buf.size());
        sendToBoth(event);

        clearExpectedComands();
        addExpectedCommand(CommandChoice::GetDescriptor()->name());

        int chosenAbilityId;
        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                chosenAbilityId = choiceCmd.choice();
                if (static_cast<size_t>(chosenAbilityId) >= e.abilities.size())
                    continue;
                break;
            }
        }
        clearExpectedComands();

        if (e.target.type == asn::TargetType::ThisCard) {
            auto card = mContext.thisCard.card;
            if (!card || card->zone()->name() != mContext.thisCard.zone)
                co_return;
            card->addAbility(e.abilities[chosenAbilityId], e.duration);
            EventAbilityGain ev;
            ev.set_zone(card->zone()->name());
            ev.set_cardid(card->pos());

            std::vector<uint8_t> abilityBuf = encodeAbility(e.abilities[chosenAbilityId]);
            ev.set_ability(abilityBuf.data(), abilityBuf.size());
            sendToBoth(ev);
        }
        co_return;
    }

    if (e.target.type == asn::TargetType::ThisCard) {
        auto card = mContext.thisCard.card;
        if (!card || card->zone()->name() != mContext.thisCard.zone)
            co_return;
        for (const auto &a: e.abilities) {
            card->addAbility(a, e.duration);
            EventAbilityGain ev;
            ev.set_zone(card->zone()->name());
            ev.set_cardid(card->pos());

            std::vector<uint8_t> abilityBuf = encodeAbility(a);
            ev.set_ability(abilityBuf.data(), abilityBuf.size());
            sendToBoth(ev);
        }
    }
}
