#include "abilities.pb.h"
#include "moveCommands.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"

namespace {
bool isInFrontOf(int backPos, int frontPos) {
    if ((backPos == 3 && (frontPos == 0 || frontPos == 1))
        || (backPos == 4 && (frontPos == 1 || frontPos == 2)))
        return true;
    return false;
}
}

Resumable AbilityPlayer::playEffect(const asn::Effect &e, std::optional<asn::Effect> nextEffect) {
    if (!evaluateCondition(e.cond)) {
        mConditionNotMet = true;
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
    case asn::EffectType::TriggerCheckTwice:
        playTriggerCheckTwice();
        break;
    case asn::EffectType::Look:
        co_await playLook(std::get<asn::Look>(e.effect), nextEffect);
        break;
    case asn::EffectType::EarlyPlay:
        playEarlyPlay();
        break;
    case asn::EffectType::PerformEffect:
        co_await playPerformEffect(std::get<asn::PerformEffect>(e.effect));
        break;
    case asn::EffectType::OtherEffect:
        co_await playOtherEffect(std::get<asn::OtherEffect>(e.effect));
        break;
    default:
        assert(false);
        break;
    }
    setMandatory(true);
}

Resumable AbilityPlayer::playEffects(const std::vector<asn::Effect> &e) {
    for (size_t i = 0; i < e.size(); ++i) {
        if (e[i].type == asn::EffectType::Look && (i != e.size() - 1))
            co_await playEffect(e[i], e[i + 1]);
        else
            co_await playEffect(e[i]);
        if (mConditionNotMet) {
            mConditionNotMet = false;
            break;
        }
    }
}

void AbilityPlayer::playContEffect(const asn::Effect &e) {
    switch (e.type) {
    case asn::EffectType::AttributeGain:
        playAttributeGain(std::get<asn::AttributeGain>(e.effect), true);
        break;
    case asn::EffectType::EarlyPlay:
        playEarlyPlay();
        break;
    case asn::EffectType::CannotPlay:
        playCannotPlay();
        break;
    default:
        break;
    }
}

Resumable AbilityPlayer::playNonMandatory(const asn::NonMandatory &e) {
    setMandatory(false);
    co_await playEffects(e.effect);
    setMandatory(true);
    bool youDo = !canceled();
    setCanceled(false);
    if (youDo)
        co_await playEffects(e.ifYouDo);
    else
        co_await playEffects(e.ifYouDont);
}

Resumable AbilityPlayer::playChooseCard(const asn::ChooseCard &e) {
    clearChosenCards();
    std::vector<uint8_t> buf;
    encodeChooseCard(e, buf);

    EventChooseCard ev;
    ev.set_effect(buf.data(), buf.size());
    ev.set_mandatory(mandatory());
    mPlayer->sendToBoth(ev);

    auto player = owner(e.executor);
    player->clearExpectedComands();
    player->addExpectedCommand(CommandChooseCard::GetDescriptor()->name());
    // TODO: check for legitimacy of cancel
    player->addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());

    while (true) {
        GameCommand cmd;
        if (mLastCommand) {
            cmd = *mLastCommand;
            mLastCommand.reset();
        } else {
            cmd = co_await waitForCommand();
        }
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
                     spec->number.value > chooseCmd.ids_size()) ||
                    (spec->number.mod == asn::NumModifier::UpTo &&
                     spec->number.value < chooseCmd.ids_size()))
                continue;
            }
            //TODO: add checks
            for (int i = chooseCmd.ids_size() - 1; i >= 0; --i) {
                auto playerType = protoPlayerToPlayer(chooseCmd.owner());
                if (e.executor == asn::Player::Opponent) {
                    // revert sides
                    if (playerType == asn::Player::Player)
                        playerType = asn::Player::Opponent;
                    else
                        playerType = asn::Player::Player;
                }
                auto pzone = owner(playerType)->zone(chooseCmd.zone());
                if (!pzone)
                    break;

                auto card = pzone->card(chooseCmd.ids(i));
                if (!card)
                    break;
                addChosenCard(CardImprint(chooseCmd.zone(), chooseCmd.ids(i), card, card->player() != mPlayer));
                if (e.placeType == asn::PlaceType::Selection)
                    removeMentionedCard(chooseCmd.ids(i));
            }
            break;
        }
    }
    player->clearExpectedComands();
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

Resumable AbilityPlayer::getStagePosition(int &position, const asn::MoveCard &e) {
    std::vector<uint8_t> buf;
    encodeMoveCard(e, buf);
    EventMoveDestinationIndexChoice ev;
    ev.set_effect(buf.data(), buf.size());
    ev.set_mandatory(true);
    mPlayer->sendToBoth(ev);

    auto player = owner(e.executor);
    player->clearExpectedComands();
    player->addExpectedCommand(CommandChoice::GetDescriptor()->name());

    int pos = 0;
    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandChoice>()) {
            CommandChoice choiceCmd;
            cmd.command().UnpackTo(&choiceCmd);
            pos = choiceCmd.choice();
            if (pos >= 5)
                continue;
            break;
        }
    }

    position = pos;
    player->clearExpectedComands();
}

Resumable AbilityPlayer::moveTopDeck(const asn::MoveCard &e, int toZoneIndex, int toIndex) {
    auto player = owner(e.from.owner);
    auto zone = player->zone(asnZoneToString(e.from.zone));
    const auto &spec = *e.target.targetSpecification;
    auto deck = player->zone("deck");
    clearLastMovedCards();
    for (int i = 0; i < spec.number.value; ++i) {
        auto card = deck->topCard();
        player->moveCard(asnZoneToString(e.from.zone), deck->count() - 1, asnZoneToString(e.to[toZoneIndex].zone), toIndex, revealChosen());
        addLastMovedCard(CardImprint(card->zone()->name(), card->pos(), card, e.to[toZoneIndex].owner == asn::Player::Opponent));

        // TODO: refresh and levelup are triggered at the same time, give choice
        if (player->zone("deck")->count() == 0)
            player->refresh();
        if (e.to[toZoneIndex].zone == asn::Zone::Clock && player->zone("clock")->count() >= 7)
            co_await player->levelUp();
    }
}

Resumable AbilityPlayer::playMoveCard(const asn::MoveCard &e) {
    if ((e.target.type == asn::TargetType::ChosenCards && chosenCards().empty()) ||
        ((e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) &&
            mentionedCards().empty()) ||
        (e.target.type == asn::TargetType::ThisCard && thisCard().zone != thisCard().card->zone()->name()))
        co_return;

    if ((e.from.pos == asn::Position::Top || e.from.pos == asn::Position::Bottom) &&
        mPlayer->zone(asnZoneToString(e.from.zone))->count() == 0)
        co_return;

    if (e.target.type == asn::TargetType::BattleOpponent) {
        if (thisCard().card->zone()->name() != "stage")
            co_return;

        auto card = mPlayer->oppositeCard(thisCard().card);
        if (!card)
            co_return;
    }

    std::map<int, ServerCard*> cardsToMove;
    bool moveCanceled = false;
    if (e.target.type == asn::TargetType::SpecificCards && e.from.pos == asn::Position::NotSpecified
        && e.target.targetSpecification->mode != asn::TargetMode::All) {
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
                moveCanceled = true;
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
                moveCanceled = true;
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
    if (moveCanceled)
        co_return;

    if (e.target.type == asn::TargetType::MentionedCards && e.order == asn::Order::Any) {
        // coming here from look effect
        assert(e.to.size() == 1);
        while (true) {
            GameCommand cmd;
            if (mLastCommand) {
                cmd = *mLastCommand;
                mLastCommand.reset();
            } else {
                cmd = co_await waitForCommand();
            }
            if (cmd.command().Is<CommandMoveInOrder>()) {
                CommandMoveInOrder moveCmd;
                cmd.command().UnpackTo(&moveCmd);
                if (mentionedCards().size() && mentionedCards()[0].card->zone()->name() == asnZoneToString(e.to[0].zone)) {
                    if (static_cast<int>(mentionedCards().size()) != moveCmd.codes_size())
                        co_return;
                    mPlayer->reorderTopCards(moveCmd, e.to[0].zone);
                    co_return;
                }
            } else if (cmd.command().Is<CommandConfirmMove>()) {
                break;
            }
        }
    }

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
    //choosing target stage position
    if (e.to[toZoneIndex].zone == asn::Zone::Stage) {
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
            if (e.target.type == asn::TargetType::LastMovedCards) {
                ServerPlayer *executor = mPlayer;
                for (const auto &card: lastMovedCards()) {
                    executor = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
                    cardsToMove[card.id] = card.card;
                }

                clearLastMovedCards();
                for (auto it = cardsToMove.rbegin(); it != cardsToMove.rend(); ++it) {
                    co_await getStagePosition(toIndex, e);
                    executor->moveCard(it->second->zone()->name(), it->first, asnZoneToString(e.to[toZoneIndex].zone), toIndex, revealChosen());
                }

                co_return;
            } else {
                co_await getStagePosition(toIndex, e);
            }
        }
    }

    ServerPlayer *player = mPlayer;
    if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: chosenCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.id] = card.card;
        }
    } else if (e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) {
        for (const auto &card: mentionedCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.id] = card.card;
        }
    } else if (e.target.type == asn::TargetType::SpecificCards) {
        if (e.from.pos == asn::Position::Top)
            co_await moveTopDeck(e, toZoneIndex, toIndex);
        player = owner(e.from.owner);
        auto zone = player->zone(asnZoneToString(e.from.zone));
        const auto &spec = *e.target.targetSpecification;
        if (e.from.pos == asn::Position::Bottom)
            cardsToMove[0] = zone->card(0);
        else if (spec.mode == asn::TargetMode::All) {
            for (int i = 0; i < zone->count(); ++i) {
                auto card = zone->card(i);
                if (!card)
                    continue;

                if (checkCard(spec.cards.cardSpecifiers, *card))
                    cardsToMove[i] = card;
            }
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
    } else if (e.target.type == asn::TargetType::BattleOpponent) {
        auto card = mPlayer->oppositeCard(thisCard().card);
        player = owner(card);
        cardsToMove[card->pos()] = card;
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
        clearMentionedCards();
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
            card.card->player()->addAttributeBuff(card.card, e.type, value, e.duration);
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
            mPlayer->addAttributeBuff(thisCard().card, e.type, value, e.duration);
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
            else if (spec.mode == asn::TargetMode::InFrontOfThis && !isInFrontOf(thisCard().card->pos(), card->pos()))
                continue;

            bool positional = false;
            if (spec.mode == asn::TargetMode::InFrontOfThis || spec.mode == asn::TargetMode::BackRow
                || spec.mode == asn::TargetMode::FrontRow)
                positional = true;

            if (spec.mode == asn::TargetMode::All || checkCard(spec.cards.cardSpecifiers, *card)) {
                if (cont) {
                    if (revert())
                        mPlayer->removeContAttributeBuff(card, thisCard().card, abilityId(), e.type);
                    else
                        mPlayer->addContAttributeBuff(card, thisCard().card, abilityId(), e.type, value, positional);
                } else {
                    mPlayer->addAttributeBuff(card, e.type, value, e.duration);
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
                mPlayer->addAttributeBuff(card, e.type, value, e.duration);
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
        co_await playEffects(e.ifYouDo);

    if (canceled())
        co_await playEffects(e.ifYouDont);
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
                e.targets[0].number.value > chooseCmd.ids_size()) ||
                (e.targets[0].number.mod == asn::NumModifier::UpTo &&
                e.targets[0].number.value < chooseCmd.ids_size()))
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
            mPlayer->addAbilityToCard(card, e.abilities[chosenAbilityId], e.duration);
        }
        co_return;
    }

    if (e.target.type == asn::TargetType::ThisCard) {
        auto card = thisCard().card;
        if (!card || card->zone()->name() != thisCard().zone)
            co_return;
        for (const auto &a: e.abilities)
            mPlayer->addAbilityToCard(card, a, e.duration);
    }
}

Resumable AbilityPlayer::playPerformEffect(const asn::PerformEffect &e) {
    if (static_cast<size_t>(e.numberOfEffects) < e.effects.size()) {
        std::vector<uint8_t> buf;
        encodePerformEffect(e, buf);

        EventEffectChoice event;
        event.set_effect(buf.data(), buf.size());
        mPlayer->sendToBoth(event);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::GetDescriptor()->name());

        int chosenEffectId;
        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                chosenEffectId = choiceCmd.choice();
                if (static_cast<size_t>(chosenEffectId) >= e.effects.size())
                    continue;
                break;
            }
        }
        mPlayer->clearExpectedComands();

        co_await playEventAbility(e.effects[chosenEffectId]);

        co_return;
    }

    for (const auto &a: e.effects)
        co_await playEventAbility(a);
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
    int count = 0, climaxCount = 0;
    int cardCount = res->count();
    for (int i = 0; i < cardCount; ++i) {
        auto card = res->card(res->count() - 1);
        if (checkCard(e.forEach.cardSpecifiers, *card))
            ++count;
        if (card->type() == asn::CardType::Climax)
            ++climaxCount;
        mPlayer->moveCard("res", res->count() - 1, "wr");
    }
    if (climaxCount)
        owner(asn::Player::Opponent)->checkOtherTrigger("KGL/S79-016");

    for (int i = 0; i < count; ++i)
        co_await playEffects(e.effect);
}

void AbilityPlayer::playBackup(const asn::Backup &e) {
    auto opponent = mPlayer->getOpponent();
    auto charInBattle = opponent->oppositeCard(opponent->attackingCard());
    mPlayer->addAttributeBuff(charInBattle, asn::AttributeType::Power, e.power, 1);
    mPlayer->checkOnBackup(thisCard().card);
}

void AbilityPlayer::playTriggerCheckTwice() {
    thisCard().card->setTriggerCheckTwice(true);
}

Resumable AbilityPlayer::playLook(const asn::Look &e, std::optional<asn::Effect> nextEffect) {
    assert(e.place.owner == asn::Player::Player);
    assert(e.place.zone == asn::Zone::Deck);
    clearMentionedCards();
    auto deck = mPlayer->zone("deck");
    if (!deck->count())
        co_return;

    if (e.number.mod == asn::NumModifier::UpTo) {
        std::vector<uint8_t> buf;
        encodeLook(e, buf);

        EventLook ev;
        ev.set_effect(buf.data(), buf.size());
        if (nextEffect) {
            ev.set_nexteffecttype(static_cast<int>(nextEffect->type));
            std::vector<uint8_t> nextBuf;
            if (nextEffect->type == asn::EffectType::MoveCard)
                encodeMoveCard(std::get<asn::MoveCard>(nextEffect->effect), nextBuf);
            else if (nextEffect->type == asn::EffectType::ChooseCard)
                encodeChooseCard(std::get<asn::ChooseCard>(nextEffect->effect), nextBuf);
            ev.set_nexteffect(nextBuf.data(), nextBuf.size());
        }
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandCancelEffect::GetDescriptor()->name());
        mPlayer->addExpectedCommand(CommandLookTopDeck::GetDescriptor()->name());
        if (nextEffect) {
            if (nextEffect->type == asn::EffectType::ChooseCard)
                mPlayer->addExpectedCommand(CommandChooseCard::GetDescriptor()->name());
            if (nextEffect->type == asn::EffectType::MoveCard) {
                mPlayer->addExpectedCommand(CommandConfirmMove::GetDescriptor()->name());
                const auto &moveEffect = std::get<asn::MoveCard>(nextEffect->effect);
                if (moveEffect.order == asn::Order::Any)
                    mPlayer->addExpectedCommand(CommandMoveInOrder::GetDescriptor()->name());
            }
        }

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandCancelEffect>()) {
                // if we have already looked at at least 1 card, than this cancel refers to the next effect
                if (mMentionedCards.size())
                    mLastCommand = cmd;
                break;
            } else if (cmd.command().Is<CommandLookTopDeck>()) {
                if (e.number.value == static_cast<int>(mMentionedCards.size()))
                    break;

                auto card = deck->card(deck->count() - 1 - static_cast<int>(mMentionedCards.size()));
                if (!card)
                    break;

                sendLookCard(card);

                if (static_cast<size_t>(deck->count()) <= mMentionedCards.size() || mMentionedCards.size() == static_cast<size_t>(e.number.value))
                    break;
            } else if (cmd.command().Is<CommandChooseCard>() ||
                       cmd.command().Is<CommandMoveInOrder>() ||
                       cmd.command().Is<CommandConfirmMove>()) {
                mLastCommand = cmd;
                break;
            }
        }
    } else {
        for (int i = 0; i < e.number.value && i < deck->count(); ++i) {
            auto card = deck->card(deck->count() - 1 - i);
            if (!card)
                break;

            sendLookCard(card);
        }
    }
}

void AbilityPlayer::sendLookCard(ServerCard *card) {
    EventLookTopDeck privateEvent;
    EventLookTopDeck publicEvent;

    privateEvent.set_code(card->code());
    mPlayer->sendGameEvent(privateEvent);
    mPlayer->game()->sendPublicEvent(publicEvent, mPlayer->id());

    addMentionedCard(CardImprint(card->zone()->name(), card->pos(), card, card->zone()->player() != mPlayer));
}

void AbilityPlayer::playEarlyPlay() {
    if (revert())
        mPlayer->removeContAttributeBuff(thisCard().card, thisCard().card, abilityId(), asn::AttributeType::Level);
    else
        mPlayer->addContAttributeBuff(thisCard().card, thisCard().card, abilityId(), asn::AttributeType::Level, -1);
}

void AbilityPlayer::playCannotPlay() {
    thisCard().card->setCannotPlay(!revert());

    EventSetCannotPlay ev;
    ev.set_handid(thisCard().card->pos());
    ev.set_cannotplay(thisCard().card->cannotPlay());
    mPlayer->sendGameEvent(ev);
}

Resumable AbilityPlayer::playOtherEffect(const asn::OtherEffect &e) {
    auto key = e.cardCode + '-' + std::to_string(e.effectId);
    if (key == "KGL/S79-020-3") {
        co_await playS79_20();
    }
}

Resumable AbilityPlayer::playS79_20() {
    auto opponent = mPlayer->getOpponent();
    auto pDeck = mPlayer->zone("deck");
    auto oDeck = opponent->zone("deck");

    EventRevealTopDeck eventPlayer;
    eventPlayer.set_code(pDeck->card(pDeck->count() - 1)->code());
    mPlayer->sendToBoth(eventPlayer);

    EventRevealTopDeck eventOpponent;
    eventOpponent.set_code(oDeck->card(oDeck->count() - 1)->code());
    opponent->sendToBoth(eventOpponent);

    auto pCard = pDeck->topCard();
    auto oCard = oDeck->topCard();

    if (pCard->level() > oCard->level()) {
        asn::ChooseCard c;
        c.placeType = asn::PlaceType::SpecificPlace;
        asn::Target t;
        t.type = asn::TargetType::SpecificCards;
        asn::TargetSpecificCards spec;
        spec.mode = asn::TargetMode::Any;
        spec.number = asn::Number{asn::NumModifier::ExactMatch, 1, std::nullopt};
        spec.cards.cardSpecifiers.push_back(asn::CardSpecifier{asn::CardSpecifierType::CardType, asn::CardType::Char});
        spec.cards.cardSpecifiers.push_back(asn::CardSpecifier{asn::CardSpecifierType::Trait, asn::Trait{"Shuchiin"}});
        t.targetSpecification = spec;
        c.executor = asn::Player::Player;
        c.targets.push_back(t);
        c.place = asn::Place{asn::Position::NotSpecified, asn::Zone::WaitingRoom, asn::Player::Player};

        co_await playChooseCard(c);

        asn::MoveCard m;
        asn::Target t2;
        t2.type = asn::TargetType::ChosenCards;
        m.target = t2;
        m.executor = asn::Player::Player;
        m.order = asn::Order::NotSpecified;
        m.from = asn::Place{asn::Position::NotSpecified, asn::Zone::WaitingRoom, asn::Player::Player};
        m.to.push_back(asn::Place{asn::Position::NotSpecified, asn::Zone::Hand, asn::Player::Player});

        co_await playMoveCard(m);
    } else if (pCard->level() < oCard->level()) {
        asn::ChooseCard c;
        asn::Target t;
        t.type = asn::TargetType::SpecificCards;
        asn::TargetSpecificCards spec;
        spec.mode = asn::TargetMode::Any;
        spec.number = asn::Number{asn::NumModifier::ExactMatch, 1, std::nullopt};
        spec.cards.cardSpecifiers.push_back(asn::CardSpecifier{asn::CardSpecifierType::CardType, asn::CardType::Char});
        t.targetSpecification = spec;
        c.placeType = asn::PlaceType::SpecificPlace;
        c.targets.push_back(t);
        c.place = asn::Place{asn::Position::NotSpecified, asn::Zone::Stage, asn::Player::Opponent};
        c.executor = asn::Player::Opponent;

        co_await playChooseCard(c);

        asn::AttributeGain a;
        asn::Target t2;
        t2.type = asn::TargetType::ChosenCards;
        a.target = t2;
        a.type = asn::AttributeType::Power;
        a.gainType = asn::ValueType::Raw;
        a.value = 5000;
        a.duration = 1;

        playAttributeGain(a);
    }
}

