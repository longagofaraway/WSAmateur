#include "abilities.pb.h"
#include "gameEvent.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"

Resumable AbilityPlayer::playEffect(const asn::Effect &e) {
    if (!evaluateCondition(e.cond)) {
        mPlayer->sendToBoth(EventConditionNotMet());
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
    case asn::EffectType::MoveWrToDeck:
        playMoveWrToDeck(std::get<asn::MoveWrToDeck>(e.effect));
        break;
    case asn::EffectType::ChangeState:
        playChangeState(std::get<asn::ChangeState>(e.effect));
        break;
    case asn::EffectType::FlipOver:
        co_await playFlipOver(std::get<asn::FlipOver>(e.effect));
        break;
    case asn::EffectType::Backup:
        playBackup(std::get<asn::Backup>(e.effect));
        break;
    default:
        assert(false);
        break;
    }
    setMandatory(true);
}

void AbilityPlayer::playContEffect(const asn::Effect &e) {
    switch (e.type) {
    case asn::EffectType::AttributeGain:
        playAttributeGain(std::get<asn::AttributeGain>(e.effect), true);
        break;
    default:
        break;
    }
}

Resumable AbilityPlayer::playNonMandatory(const asn::NonMandatory &e) {
    setMandatory(false);
    for (const auto &effect: e.effect)
        co_await playEffect(effect);
    setMandatory(true);
    setCanceled(false);
}

Resumable AbilityPlayer::playChooseCard(const asn::ChooseCard &e) {
    std::vector<uint8_t> buf;
    encodeChooseCard(e, buf);

    EventChooseCard ev;
    ev.set_effect(buf.data(), buf.size());
    ev.set_mandatory(mandatory());
    mPlayer->sendToBoth(ev);

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandChooseCard::GetDescriptor()->name());
    // TODO: check for legitimacy of cancel
    mPlayer->addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            setCanceled(true);
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
                auto pzone = owner(protoPlayerToPlayer(chooseCmd.owner()))->zone(chooseCmd.zone());
                if (!pzone)
                    break;

                auto card = pzone->card(chooseCmd.ids(i));
                if (!card)
                    break;
                addChosenCard(CardImprint(chooseCmd.zone(), chooseCmd.ids(i), card, chooseCmd.owner() == ProtoOwner::ProtoOpponent));
            }
            break;
        }
    }
    mPlayer->clearExpectedComands();
}

std::map<int, ServerCard*> AbilityPlayer::processCommandChooseCard(const CommandChooseCard &cmd) {
    //TODO: add checks
    std::map<int, ServerCard*> res;
    for (int i = cmd.ids_size() - 1; i >= 0; --i) {
        auto pzone = owner(protoPlayerToPlayer(cmd.owner()))->zone(cmd.zone());
        if (!pzone)
            break;

        auto card = pzone->card(cmd.ids(i));
        if (!card)
            break;

        res[cmd.ids(i)] = card;
    }
    return res;
}

Resumable AbilityPlayer::playMoveCard(const asn::MoveCard &e) {
    if ((e.target.type == asn::TargetType::ChosenCards && chosenCards().empty()) ||
        ((e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) &&
            mentionedCards().empty()) ||
        (e.target.type == asn::TargetType::ThisCard && thisCard().zone != thisCard().card->zone()->name()))
        co_return;

    if (e.from.pos == asn::Position::Top && mPlayer->zone(asnZoneToString(e.from.zone))->count() == 0)
        co_return;

    bool chooseCards = (e.target.type == asn::TargetType::SpecificCards && e.from.pos == asn::Position::NotSpecified);
    std::map<int, ServerCard*> cardsToMove;
    if (chooseCards) {
        assert(e.to.size() == 1);
        assert(e.executor == asn::Player::Player);
        std::vector<uint8_t> buf;
        encodeMoveCard(e, buf);

        EventMoveTargetChoice ev;
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(mandatory());
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChooseCard::GetDescriptor()->name());
        // TODO: check for legitimacy of cancel
        mPlayer->addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandCancelEffect>()) {
                setCanceled(true);
                break;
            } else if (cmd.command().Is<CommandChooseCard>()) {
                CommandChooseCard cardCmd;
                cmd.command().UnpackTo(&cardCmd);
                cardsToMove = processCommandChooseCard(cardCmd);
                break;
            }
        }
        mPlayer->clearExpectedComands();
    } else if (!mandatory()) {
        assert(e.executor == asn::Player::Player);
        std::vector<uint8_t> buf;
        encodeMoveCard(e, buf);

        EventMoveChoice ev;
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(mandatory());
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::GetDescriptor()->name());
        // TODO: check for legitimacy of cancel
        mPlayer->addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandCancelEffect>()) {
                setCanceled(true);
                break;
            } else if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                // 0 is yes, 'yes' will be the first choice on client's side
                setCanceled(choiceCmd.choice());
                break;
            }
        }
        mPlayer->clearExpectedComands();
    }
    if (canceled())
        co_return;

    // choice of a destination
    int toZoneIndex = 0;
    if (e.to.size() > 1) {
        assert(e.executor == asn::Player::Player);
        assert(mandatory());
        std::vector<uint8_t> buf;
        encodeMoveCard(e, buf);
        EventMoveDestinationChoice ev;
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(true);
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::GetDescriptor()->name());

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                toZoneIndex = choiceCmd.choice();
                if (static_cast<size_t>(toZoneIndex) >= e.to.size())
                    continue;
                break;
            }
        }
        mPlayer->clearExpectedComands();
    }

    int toIndex = 0;
    if (e.to[toZoneIndex].pos == asn::Position::EmptySlotBackRow ||
        (e.to[toZoneIndex].zone == asn::Zone::Stage && e.to[toZoneIndex].pos == asn::Position::NotSpecified)) {
        assert(mandatory());
        bool positionSet = false;
        if (e.to[toZoneIndex].pos == asn::Position::EmptySlotBackRow) {
            auto player = owner(e.to[toZoneIndex].owner);
            auto stage = player->zone("stage");
            if (stage->card(3) && !stage->card(4)) {
                positionSet = true;
                toIndex = 4;
            } else if (!stage->card(3) && stage->card(4)) {
                positionSet = true;
                toIndex = 3;
            } else if (stage->card(3) && stage->card(4)) {
                co_return;
            }
        }
        if (!positionSet) {
            std::vector<uint8_t> buf;
            encodeMoveCard(e, buf);
            EventMoveDestinationIndexChoice ev;
            ev.set_effect(buf.data(), buf.size());
            ev.set_mandatory(true);
            mPlayer->sendToBoth(ev);

            auto player = owner(e.executor);
            player->clearExpectedComands();
            player->addExpectedCommand(CommandChoice::GetDescriptor()->name());

            while (true) {
                auto cmd = co_await waitForCommand();
                if (cmd.command().Is<CommandChoice>()) {
                    CommandChoice choiceCmd;
                    cmd.command().UnpackTo(&choiceCmd);
                    toIndex = choiceCmd.choice();
                    if (toIndex >= 5)
                        continue;
                    break;
                }
            }
            player->clearExpectedComands();
        }
    }

    ServerPlayer *player = mPlayer;
    if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: chosenCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.id] = card.card;
        }
    } else if (e.target.type == asn::TargetType::SpecificCards) {
        if (e.from.pos == asn::Position::Top) {
            player = owner(e.from.owner);
            auto zone = mPlayer->zone(asnZoneToString(e.from.zone));
            cardsToMove[zone->count() - 1] = zone->card(zone->count() - 1);
        }
    } else if (e.target.type == asn::TargetType::ThisCard) {
        if (e.to[toZoneIndex].pos == asn::Position::SlotThisWasIn)
            toIndex = thisCard().card->prevStagePos();
        cardsToMove[thisCard().id] = thisCard().card;
    } else if (e.target.type == asn::TargetType::LastMovedCards) {
        for (const auto &card: lastMovedCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.id] = card.card;
        }
    }

    clearLastMovedCards();
    for (auto it = cardsToMove.rbegin(); it != cardsToMove.rend(); ++it) {
        player->moveCard(it->second->zone()->name(), it->first, asnZoneToString(e.to[toZoneIndex].zone), toIndex, revealChosen());
        addLastMovedCard(CardImprint(it->second->zone()->name(), it->second->pos(), it->second, e.to[toZoneIndex].owner == asn::Player::Opponent));

        if (e.to[toZoneIndex].pos == asn::Position::SlotThisWasIn)
            player->setCardState(it->second, CardState::StateRested);

        // TODO: refresh and levelup are triggered at the same time, give choice
        if (player->zone("deck")->count() == 0)
            player->refresh();
        if (e.to[toZoneIndex].zone == asn::Zone::Clock && player->zone("clock")->count() >= 7)
            co_await player->levelUp();
    }
}

Resumable AbilityPlayer::playDrawCard(const asn::DrawCard &e) {
    bool confirmed = mandatory();
    if (!mandatory()) {
        std::vector<uint8_t> buf;
        encodeDrawCard(e, buf);

        EventDrawChoice ev;
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(false);
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::GetDescriptor()->name());

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
        mPlayer->clearExpectedComands();
    }
    if (!confirmed)
        co_return;
    mPlayer->moveTopDeck("hand");
}

void AbilityPlayer::playRevealCard(const asn::RevealCard &e) {
    switch (e.type) {
    case asn::RevealType::TopDeck:
        if (e.number.mod == asn::NumModifier::ExactMatch) {
            for (int i = 0; i < e.number.value; ++i) {
                auto deck = mPlayer->zone("deck");
                if (i >= deck->count())
                    break;

                EventRevealTopDeck event;
                event.set_code(deck->card(deck->count() - i - 1)->code());
                mPlayer->sendToBoth(event);
                addMentionedCard(CardImprint("deck", deck->count() - i - 1, deck->card(deck->count() - i - 1)));
            }
        } else {
            assert(false);
        }
        break;
    case asn::RevealType::ChosenCards:
        setRevealChosen(true);
        break;
    default:
        assert(false);
        break;
    }
}

void AbilityPlayer::playAttributeGain(const asn::AttributeGain &e, bool cont) {
    int value = e.value;
    if (e.gainType == asn::ValueType::Multiplier) {
        assert(e.modifier->type == asn::MultiplierType::ForEach);
        assert(e.modifier->forEach->type == asn::TargetType::SpecificCards);
        auto pzone = mPlayer->zone(asnZoneToString(e.modifier->zone));
        int cardCount = 0;
        for (int i = 0; i < pzone->count(); ++i) {
            auto card = pzone->card(i);
            if (!card)
                continue;

            const auto &tspec = *e.modifier->forEach->targetSpecification;
            if (tspec.mode == asn::TargetMode::AllOther && card == thisCard().card)
                continue;

            if (checkCard(tspec.cards.cardSpecifiers, *card))
                cardCount++;
        }

        value = cardCount * e.value;
    }

    if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: chosenCards())
            mPlayer->addAttributeBuff(e.type, card.id, value, e.duration);
    } else if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().card == nullptr ||
            (thisCard().card->zone()->name() != "stage" && thisCard().card->zone()->name() != "climax"))
            return;

        if (cont) {
            if (revert())
                mPlayer->removeContAttributeBuff(thisCard().card, thisCard().card, abilityId(), e.type);
            else
                mPlayer->addContAttributeBuff(thisCard().card, thisCard().card, abilityId(), e.type, value);
        } else {
            mPlayer->addAttributeBuff(e.type, thisCard().id, value, e.duration);
        }
    } else if (e.target.type == asn::TargetType::SpecificCards) {
        const auto &spec = *e.target.targetSpecification;
        auto stage = mPlayer->zone("stage");
        for (int i = 0; i < stage->count(); ++i) {
            auto card = stage->card(i);
            if (!card)
                continue;

            if (spec.mode == asn::TargetMode::AllOther && card == thisCard().card)
                continue;

            if (spec.mode == asn::TargetMode::All || checkCard(spec.cards.cardSpecifiers, *card)) {
                if (cont) {
                    if (revert())
                        mPlayer->removeContAttributeBuff(card, thisCard().card, abilityId(), e.type);
                    else
                        mPlayer->addContAttributeBuff(card, thisCard().card, abilityId(), e.type, value);
                } else {
                    mPlayer->addAttributeBuff(e.type, thisCard().id, value, e.duration);
                }
            }
        }
    } else if (e.target.type == asn::TargetType::OppositeThis) {
        auto card = mPlayer->oppositeCard(thisCard().card);
        if (card) {
            if (cont) {
                if (revert())
                    mPlayer->removeContAttributeBuff(card, thisCard().card, abilityId(), e.type);
                else
                    mPlayer->addContAttributeBuff(card, thisCard().card, abilityId(), e.type, value, true);
            } else {
                mPlayer->game()->opponentOfPlayer(mPlayer->id())->addAttributeBuff(e.type, thisCard().id, value, e.duration);
            }
        }
    }
}

Resumable AbilityPlayer::playPayCost(const asn::PayCost &e) {
    mPlayer->sendToBoth(EventPayCost());

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandPlayEffect::GetDescriptor()->name());
    mPlayer->addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            setCanceled(true);
            break;
        } else if (cmd.command().Is<CommandPlayEffect>()) {
            co_await payCost();
            break;
        }
    }
    mPlayer->clearExpectedComands();

    if (!canceled())
        for (const auto &effect: e.ifYouDo)
            co_await playEffect(effect);

    if (canceled())
        for (const auto &effect: e.ifYouDont)
            co_await playEffect(effect);
    co_return;
}


Resumable AbilityPlayer::playSearchCard(const asn::SearchCard &e) {
    std::vector<uint8_t> buf;
    encodeSearchCard(e, buf);

    EventSearchCard eventPrivate;
    eventPrivate.set_effect(buf.data(), buf.size());
    EventSearchCard eventPublic(eventPrivate);
    auto pzone = mPlayer->zone(asnZoneToString(e.place.zone));
    for (int i = 0; i < pzone->count(); ++i)
        eventPrivate.add_codes(pzone->card(i)->code());
    for (int i = 0; i < pzone->count(); ++i)
        eventPublic.add_codes("");
    mPlayer->sendGameEvent(eventPrivate);
    mPlayer->game()->sendPublicEvent(eventPublic, mPlayer->id());

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandChooseCard::GetDescriptor()->name());
    // TODO: check for legitimacy of cancel
    mPlayer->addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

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
                auto pzone = owner(protoPlayerToPlayer(chooseCmd.owner()))->zone(chooseCmd.zone());
                if (!pzone)
                    break;

                auto card = pzone->card(chooseCmd.ids(i));
                if (!card)
                    break;
                addChosenCard(CardImprint(chooseCmd.zone(), chooseCmd.ids(i), card, chooseCmd.owner() == ProtoOwner::ProtoOpponent));
            }
            break;
        }
    }
    mPlayer->clearExpectedComands();
}

void AbilityPlayer::playShuffle(const asn::Shuffle &e) {
    assert(e.owner != asn::Player::Both);
    assert(e.owner != asn::Player::NotSpecified);
    owner(e.owner)->zone(asnZoneToString(e.zone))->shuffle();
}

Resumable AbilityPlayer::playAbilityGain(const asn::AbilityGain &e) {
    if (static_cast<size_t>(e.number) < e.abilities.size()) {
        std::vector<uint8_t> buf;
        encodeAbilityGain(e, buf);

        EventAbilityChoice event;
        event.set_effect(buf.data(), buf.size());
        mPlayer->sendToBoth(event);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::GetDescriptor()->name());

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
        mPlayer->clearExpectedComands();

        if (e.target.type == asn::TargetType::ThisCard) {
            auto card = thisCard().card;
            if (!card || card->zone()->name() != thisCard().zone)
                co_return;
            card->addAbility(e.abilities[chosenAbilityId], e.duration);
            EventAbilityGain ev;
            ev.set_zone(card->zone()->name());
            ev.set_cardid(card->pos());

            std::vector<uint8_t> abilityBuf = encodeAbility(e.abilities[chosenAbilityId]);
            ev.set_ability(abilityBuf.data(), abilityBuf.size());
            mPlayer->sendToBoth(ev);
        }
        co_return;
    }

    if (e.target.type == asn::TargetType::ThisCard) {
        auto card = thisCard().card;
        if (!card || card->zone()->name() != thisCard().zone)
            co_return;
        for (const auto &a: e.abilities) {
            card->addAbility(a, e.duration);
            EventAbilityGain ev;
            ev.set_zone(card->zone()->name());
            ev.set_cardid(card->pos());

            std::vector<uint8_t> abilityBuf = encodeAbility(a);
            ev.set_ability(abilityBuf.data(), abilityBuf.size());
            mPlayer->sendToBoth(ev);
        }
    }
}

void AbilityPlayer::playMoveWrToDeck(const asn::MoveWrToDeck &e) {
    if (e.executor == asn::Player::Player || e.executor == asn::Player::Both) {
        mPlayer->moveWrToDeck();
        mPlayer->sendToBoth(EventRefresh());
    }
    if (e.executor == asn::Player::Opponent || e.executor == asn::Player::Both) {
        auto opponent = mPlayer->game()->opponentOfPlayer(mPlayer->id());
        opponent->moveWrToDeck();
        opponent->sendToBoth(EventRefresh());
    }
}

void AbilityPlayer::playChangeState(const asn::ChangeState &e) {
    if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().zone != thisCard().card->zone()->name())
            return;
        mPlayer->setCardState(thisCard().card, stateToProtoState(e.state));
    } else {
        assert(false);
    }
}

Resumable AbilityPlayer::playFlipOver(const asn::FlipOver &e) {
    assert(e.number.mod == asn::NumModifier::ExactMatch);
    for (int i = 0; i < e.number.value; ++i)
        mPlayer->moveTopDeck("res");

    auto res = mPlayer->zone("res");
    int count = 0;
    for (int i = 0; i < e.number.value; ++i) {
        auto card = res->card(res->count() - 1);
        if (checkCard(e.forEach.cardSpecifiers, *card))
            ++count;
        mPlayer->moveCard("res", res->count() - 1, "wr");
    }

    for (int i = 0; i < count; ++i)
        for (const auto &effect: e.effect)
            co_await playEffect(effect);
}

void AbilityPlayer::playBackup(const asn::Backup &e) {
    auto opponent = mPlayer->getOpponent();
    auto charInBattle = opponent->oppositeCard(opponent->attackingCard());
    mPlayer->addAttributeBuff(asn::AttributeType::Power, charInBattle->pos(), e.power, 1);
    mPlayer->checkOnBackup(thisCard().card);
}
