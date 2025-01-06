#include "abilityPlayer.h"

#include <QDebug>

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"

#include "abilityUtils.h"
#include "codecs/encode.h"
#include "serverPlayer.h"

namespace {

bool isStandbyTarget(const asn::Target &target) {
    if (target.type != asn::TargetType::SpecificCards)
        return false;
    const auto &spec = target.targetSpecification.value();
    return std::any_of(spec.cards.cardSpecifiers.begin(), spec.cards.cardSpecifiers.end(),
                       [](const auto &cardSpec) { return cardSpec.type == asn::CardSpecifierType::StandbyTarget; });
}

bool shoulLogMoveOnClient(const asn::MoveCard &e) {
    // player will see the whole move in another window and confirm it
    // otherwise it is difficult to see, what cards were moved
    if (e.target.type == asn::TargetType::SpecificCards &&
        (e.from.pos == asn::Position::Top || e.from.pos == asn::Position::Bottom) &&
        e.from.zone == asn::Zone::Deck && e.to[0].zone == asn::Zone::WaitingRoom)
        return true;
    return false;
}

void sendStartLog(ServerPlayer *player) {
    player->sendToBoth(EventStartMoveLog());
}

Resumable endMoveLog(ServerPlayer *player) {
    player->clearExpectedComands();
    player->addExpectedCommand(CommandPlayEffect::descriptor()->name());

    player->sendToBoth(EventEndMoveLog());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandPlayEffect>())
            break;
    }

    EventEndMoveLog event;
    event.set_is_confirmed(true);
    player->sendToBoth(event);
}

}

void AbilityPlayer::logMove(ServerPlayer *player, asn::Zone toZone) {
    mMoveLog.push_back({player, toZone});
}

Resumable AbilityPlayer::getStagePosition(int &position, const asn::RemoveMarker &e) {
    std::vector<uint8_t> buf;
    encodeRemoveMarker(e, buf);
    co_await getStagePosition(position, buf, asn::EffectType::RemoveMarker, asn::Player::Player);
}

Resumable AbilityPlayer::getStagePosition(int &position, const asn::MoveCard &e) {
    std::vector<uint8_t> buf;
    encodeMoveCard(e, buf);
    co_await getStagePosition(position, buf, asn::EffectType::MoveCard, e.executor);
}

Resumable AbilityPlayer::getStagePosition(int &position, std::vector<uint8_t> &buf,
                                          asn::EffectType effectType, asn::Player executor) {
    EventMoveDestinationIndexChoice ev;
    ev.set_effect(buf.data(), buf.size());
    ev.set_mandatory(true);
    ev.set_effect_type(static_cast<int>(effectType));
    mPlayer->sendToBoth(ev);

    auto player = owner(executor);
    player->clearExpectedComands();
    player->addExpectedCommand(CommandChoice::descriptor()->name());

    int pos = 0;
    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandChoice>()) {
            CommandChoice choiceCmd;
            cmd.command().UnpackTo(&choiceCmd);
            pos = choiceCmd.choice();
            if (pos >= 5) {
                qInfo() << "got wrong stage position " << pos << " for effect type " << static_cast<int>(effectType);
                continue;
            }
            break;
        }
    }

    position = pos;
    player->clearExpectedComands();
}

Resumable AbilityPlayer::moveFromTop(const asn::MoveCard &e, int toZoneIndex, int toIndex) {
    auto player = owner(e.from.owner);
    auto pzone = player->zone(e.from.zone);
    const auto &spec = *e.target.targetSpecification;
    assert(spec.number.mod == asn::NumModifier::ExactMatch);

    if (!isPayingCost())
        clearLastMovedCards();

    bool clientLogEnabled = shoulLogMoveOnClient(e);
    if (clientLogEnabled)
        sendStartLog(mPlayer);

    int movedCount = 0;
    for (int i = 0; i < spec.number.value; ++i) {
        auto card = (e.from.pos == asn::Position::Top) ? pzone->topCard() : pzone->card(0);
        if (!card)
            break;

        movedCount++;
        player->moveCard(asnZoneToString(e.from.zone), card->pos(), asnZoneToString(e.to[toZoneIndex].zone),
                         MoveParams{.targetPos = toIndex, .reveal = revealChosen()});
        if (!isPayingCost())
            addLastMovedCard(CardImprint(card->zone()->name(), card, e.to[toZoneIndex].owner == asn::Player::Opponent));

        if (e.from.zone == asn::Zone::Deck || e.to[toZoneIndex].zone == asn::Zone::Clock)
            co_await player->checkRefreshAndLevelUp();
    }
    if ((spec.number.mod == asn::NumModifier::AtLeast || spec.number.mod == asn::NumModifier::ExactMatch) &&
        movedCount < spec.number.value)
        mPerformedInFull = false;

    if (clientLogEnabled)
        co_await endMoveLog(mPlayer);
}

Resumable AbilityPlayer::playMoveCard(const asn::MoveCard &e) {
    if ((e.target.type == asn::TargetType::ChosenCards && chosenCards().empty()) ||
        ((e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) &&
            mentionedCards().empty()) ||
        (e.target.type == asn::TargetType::ThisCard && thisCard().zone != thisCard().card->zone()->name())) {
        mPerformedInFull = false;
        co_return;
    }

    if (e.target.type == asn::TargetType::BattleOpponent) {
        if (thisCard().card->zone()->name() != "stage") {
            mPerformedInFull = false;
            co_return;
        }

        auto card = mPlayer->oppositeCard(thisCard().card);
        if (!card) {
            mPerformedInFull = false;
            co_return;
        }
    }

    std::map<int, ServerCard*> cardsToMove;
    // 'canceled' flag could be set in another effect, and it serves other purposes
    // we'll use local flag
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
        mPlayer->addExpectedCommand(CommandChooseCard::descriptor()->name());
        // TODO: check for legitimacy of cancel
        mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());

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
        const auto &number = e.target.targetSpecification->number;
        if ((number.mod == asn::NumModifier::AtLeast || number.mod == asn::NumModifier::ExactMatch) &&
            cardsToMove.size() < number.value)
            mPerformedInFull = false;
        mPlayer->clearExpectedComands();
    } else if (!mandatory()) {
        auto executor = owner(e.executor);
        std::vector<uint8_t> buf;
        if (e.executor == asn::Player::Opponent) {
            auto effectCopy = e;
            // for translation purposes
            effectCopy.executor = asn::Player::Player;
            effectCopy.from.owner = reversePlayer(effectCopy.from.owner);
            for (auto &to: effectCopy.to)
                to.owner = reversePlayer(to.owner);
            encodeMoveCard(effectCopy, buf);
        } else {
            encodeMoveCard(e, buf);
        }

        EventMoveChoice ev;
        ev.set_effect_type(static_cast<int>(asn::EffectType::MoveCard));
        ev.set_effect(buf.data(), buf.size());
        executor->sendToBoth(ev);

        executor->clearExpectedComands();
        executor->addExpectedCommand(CommandChoice::descriptor()->name());
        // TODO: check for legitimacy of cancel
        executor->addExpectedCommand(CommandCancelEffect::descriptor()->name());

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
                int choice = choiceCmd.choice();
                if (choice) {
                    mPerformedInFull = false;
                    setCanceled(true);
                    moveCanceled = true;
                }
                break;
            }
        }
        executor->clearExpectedComands();
    }
    if (moveCanceled)
        co_return;

    if (e.target.type == asn::TargetType::MentionedCards && e.order == asn::Order::Any &&
            e.to[0].zone == asn::Zone::Deck) {
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
                    clearMentionedCards();
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
        mPlayer->addExpectedCommand(CommandChoice::descriptor()->name());

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

    int toIndex = -1;
    //choosing target stage position
    if (e.to[toZoneIndex].zone == asn::Zone::Stage) {
        bool positionSet = false;
        if (e.to[toZoneIndex].pos == asn::Position::EmptySlotFrontRow) {
            auto player = owner(e.to[toZoneIndex].owner);
            auto stage = player->zone("stage");
            if (!stage->card(0) && stage->card(1) && stage->card(2)) {
                positionSet = true;
                toIndex = 0;
            } else if (stage->card(0) && !stage->card(1) && stage->card(2)) {
                positionSet = true;
                toIndex = 1;
            } else if (stage->card(0) && stage->card(1) && !stage->card(2)) {
                positionSet = true;
                toIndex = 2;
            } else if (stage->card(0) && stage->card(1) && stage->card(2)) {
                mPerformedInFull = false;
                co_return;
            }
        } else if (e.to[toZoneIndex].pos == asn::Position::EmptySlotBackRow) {
            auto player = owner(e.to[toZoneIndex].owner);
            auto stage = player->zone("stage");
            if (stage->card(3) && !stage->card(4)) {
                positionSet = true;
                toIndex = 4;
            } else if (!stage->card(3) && stage->card(4)) {
                positionSet = true;
                toIndex = 3;
            } else if (stage->card(3) && stage->card(4)) {
                mPerformedInFull = false;
                co_return;
            }
        } else if (e.to[toZoneIndex].pos == asn::Position::SlotThisWasIn ||
                   e.to[toZoneIndex].pos == asn::Position::SlotThisWasInRested) {
            positionSet = true;
            toIndex = thisCard().card->prevStagePos();
        } else if (e.to[toZoneIndex].pos == asn::Position::SlotTargetWasIn) {
            positionSet = true;
            auto targets = getTargets(e.target);
            if (targets.empty())
                qDebug() << "wasn't able to get toIndex for SlotThisWasIn";
            else
                toIndex = targets.front()->prevStagePos();
            if (targets.size() > 1)
                qDebug() << "using slotTargetWasIn for multiple targets";
        } else if (e.to[toZoneIndex].pos == asn::Position::EmptyFrontRowMiddlePosition) {
            auto player = owner(e.to[toZoneIndex].owner);
            auto stage = player->zone("stage");
            if (!stage->card(1)) {
                positionSet = true;
                toIndex = 1;
            } else {
                mPerformedInFull = false;
                co_return;
            }
        }
        if (!positionSet) {
            if (e.target.type == asn::TargetType::LastMovedCards) {
                ServerPlayer *executor = mPlayer;
                for (const auto &card: lastMovedCards()) {
                    executor = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
                    cardsToMove[card.card->pos()] = card.card;
                }

                clearLastMovedCards();
                for (auto it = cardsToMove.rbegin(); it != cardsToMove.rend(); ++it) {
                    co_await getStagePosition(toIndex, e);
                    executor->moveCard(it->second->zone()->name(), it->first, asnZoneToString(e.to[toZoneIndex].zone),
                                       MoveParams{.targetPos = toIndex, .reveal = revealChosen()});
                }

                co_return;
            } else {
                co_await getStagePosition(toIndex, e);
            }
        }
    } else if (e.to[toZoneIndex].pos == asn::Position::Bottom) {
        toIndex = 0;
    } else if (e.to[toZoneIndex].zone == asn::Zone::Deck && e.target.type == asn::TargetType::ChosenCards
               && chosenCards().size() > 0 && chosenCards().front().card->zone()->name() == "deck") {
        // when we are looking at the top cards of the deck and moving some of them
        // back on top of the deck, mentioned cards will remain on top of the deck
        auto player = owner(e.to[toZoneIndex].owner);
        if (mentionedCards().size() > 0 && mentionedCards().front().card->player() == player) {
            // -1 - excluding moved card itself
            toIndex = player->zone(e.to[toZoneIndex].zone)->count() - 1 - mentionedCards().size();
        }
    }

    ServerPlayer *player = mPlayer;
    if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: chosenCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.card->pos()] = card.card;
        }
    } else if (e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) {
        for (const auto &card: mentionedCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.card->pos()] = card.card;
        }
    } else if (e.target.type == asn::TargetType::SpecificCards) {
        // can't process top/bottom cards of deck in the main cycle below,
        // because we don't know ids of next cards in case of refresh
        if (e.from.pos == asn::Position::Top || e.from.pos == asn::Position::Bottom) {
            co_await moveFromTop(e, toZoneIndex, toIndex);
            co_return;
        }
        player = owner(e.from.owner);
        auto zone = player->zone(e.from.zone);
        const auto &spec = *e.target.targetSpecification;
        if (spec.mode == asn::TargetMode::All) {
            for (int i = 0; i < zone->count(); ++i) {
                auto card = zone->card(i);
                if (!card)
                    continue;

                if (checkCard(spec.cards.cardSpecifiers, *card))
                    cardsToMove[i] = card;
            }
        }
    } else if (e.target.type == asn::TargetType::ThisCard) {
        cardsToMove[thisCard().card->pos()] = thisCard().card;
    } else if (e.target.type == asn::TargetType::LastMovedCards) {
        for (const auto &card: lastMovedCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.card->pos()] = card.card;
        }
    } else if (e.target.type == asn::TargetType::BattleOpponent) {
        auto card = mPlayer->oppositeCard(thisCard().card);
        player = owner(card);
        cardsToMove[card->pos()] = card;
    } else if (e.target.type == asn::TargetType::MentionedInTrigger) {
        if (cardFromTrigger())
            cardsToMove[cardFromTrigger()->pos()] = cardFromTrigger();
    }

    if (!isPayingCost())
        clearLastMovedCards();
    if (!cardsToMove.empty() && isStandbyTarget(e.target) &&
        mTriggerIcon && mTriggerIcon.value() == asn::TriggerIcon::Standby) {
        owner(asn::Player::Opponent)->triggerOnOppCharPlacedByStandby();
    }
    for (auto it = cardsToMove.rbegin(); it != cardsToMove.rend(); ++it) {
        player->moveCard(it->second->zone()->name(), it->first, asnZoneToString(e.to[toZoneIndex].zone),
                         MoveParams{.targetPos = toIndex, .reveal = revealChosen()});

        removeMentionedCard(it->second);
        if (!isPayingCost()) {
            addLastMovedCard(CardImprint(it->second->zone()->name(), it->second, e.to[toZoneIndex].owner == asn::Player::Opponent));
            logMove(player, e.to[toZoneIndex].zone);
        }

        if (e.to[toZoneIndex].pos == asn::Position::SlotThisWasInRested)
            player->setCardState(it->second, asn::State::Rested);

        if (e.from.zone == asn::Zone::Deck || e.to[toZoneIndex].zone == asn::Zone::Clock ||
            player->zone("deck")->count() == 0)
            co_await player->checkRefreshAndLevelUp();
    }

}

Resumable AbilityPlayer::playAddMarker(const asn::AddMarker &e) {
    if ((e.target.type == asn::TargetType::ChosenCards && chosenCards().empty()) ||
        ((e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) &&
            mentionedCards().empty()) ||
        (e.target.type == asn::TargetType::ThisCard && thisCard().zone != thisCard().card->zone()->name())) {
        mPerformedInFull = false;
        co_return;
    }

    if (e.target.type == asn::TargetType::SpecificCards && e.from.pos == asn::Position::NotSpecified
        && e.target.targetSpecification->mode != asn::TargetMode::All) {
        // todo: implement choice of marker
        assert(false);
    }

    // TODO: add choice of target stage cards
    auto targetStageCards = getTargets(e.destination);
    if (targetStageCards.empty()) {
        mPerformedInFull = false;
        co_return;
    }

    if (!mandatory()) {
        // TODO: same block as in playMoveCard
        std::vector<uint8_t> buf;
        encodeAddMarker(e, buf);

        EventMoveChoice ev;
        ev.set_effect_type(static_cast<int>(asn::EffectType::AddMarker));
        ev.set_effect(buf.data(), buf.size());
        auto print_context = ev.mutable_print_context();
        print_context->set_mentioned_cards_count(mentionedCards().size());
        print_context->set_last_moved_cards_count(lastMovedCards().size());
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::descriptor()->name());
        // TODO: check for legitimacy of cancel
        mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandCancelEffect>()) {
                setCanceled(true);
                break;
            } else if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                // 0 is yes, 'yes' will be the first choice on client's side
                int choice = choiceCmd.choice();
                if (choice) {
                    setCanceled(true);
                }
                break;
            }
        }
        mPlayer->clearExpectedComands();
    }
    if (canceled()) {
        mPerformedInFull = false;
        co_return;
    }

    auto targetStageCard = targetStageCards.front();
    auto player = targetStageCard->player();

    int movedCount = 0;
    if (e.target.type == asn::TargetType::SpecificCards &&
            (e.from.pos == asn::Position::Top || e.from.pos == asn::Position::Bottom)) {
        if (!isPayingCost())
            clearLastMovedCards();
        const auto &spec = *e.target.targetSpecification;
        assert(spec.number.mod == asn::NumModifier::ExactMatch);
        auto pzone = player->zone(e.from.zone);

        for (int i = 0; i < spec.number.value; ++i) {
            auto card = (e.from.pos == asn::Position::Top) ? pzone->topCard() : pzone->card(0);
            if (!card)
                break;

            movedCount++;
            player->addMarker(pzone, pzone->count() - 1, targetStageCard->pos(), e.orientation, e.withMarkers);
            if (!isPayingCost()) {
                addLastMovedCard(CardImprint(card->zone()->name(), card));
                // logMove?
            }

            if (e.from.zone == asn::Zone::Deck)
                co_await player->checkRefreshAndLevelUp();
        }

        if ((spec.number.mod == asn::NumModifier::AtLeast || spec.number.mod == asn::NumModifier::ExactMatch) &&
            movedCount < spec.number.value) {
            mPerformedInFull = false;
        }

        co_return;
    }

    // TODO: won't work with TargetType::SpecificCards
    auto targets = getTargets(e.target);
    std::sort(targets.begin(), targets.end(), [](const ServerCard *card1, const ServerCard * card2) {
        return card1->pos() > card2->pos();
    });
    if (!isPayingCost())
        clearLastMovedCards();
    for (auto target: targets) {
        auto zone = target->zone();
        player->addMarker(zone, target->pos(), targetStageCard->pos(), e.orientation, e.withMarkers);

        removeMentionedCard(target);
        if (!isPayingCost()) {
            addLastMovedCard(CardImprint(target->zone()->name(), target));
            // logMove?
        }

        if (e.from.zone == asn::Zone::Deck && target->zone()->count() == 0)
            co_await player->refresh();
    }
}

// user can't choose markers to remove atm
Resumable AbilityPlayer::playRemoveMarker(const asn::RemoveMarker &e) {
    auto targetStageCards = getTargets(e.markerBearer);
    if (targetStageCards.empty()) {
        mPerformedInFull = false;
        co_return;
    }

    // no choice for now
    assert(targetStageCards.size() == 1);
    auto markerBearer = targetStageCards.front();

    if (!mandatory()) {
        // TODO: same block as in playMoveCard
        std::vector<uint8_t> buf;
        encodeRemoveMarker(e, buf);

        EventMoveChoice ev;
        ev.set_effect_type(static_cast<int>(asn::EffectType::RemoveMarker));
        ev.set_effect(buf.data(), buf.size());
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::descriptor()->name());
        // TODO: check for legitimacy of cancel
        mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandCancelEffect>()) {
                setCanceled(true);
                break;
            } else if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                // 0 is yes, 'yes' will be the first choice on client's side
                int choice = choiceCmd.choice();
                if (choice) {
                    setCanceled(true);
                }
                break;
            }
        }
        mPlayer->clearExpectedComands();
    }
    if (canceled()) {
        mPerformedInFull = false;
        co_return;
    }

    if (!isPayingCost())
        clearLastMovedCards();

    if (e.targetMarker.type == asn::TargetType::SpecificCards) {
        const auto &spec = *e.targetMarker.targetSpecification;
        int count{0};
        const auto &markers = markerBearer->markers();
        for (size_t i = markers.size(); i > 0; --i) {
            if (!markers[i-1])
                continue;
            if (markers[i-1]->faceOrientation() == asn::FaceOrientation::FaceDown &&
                    spec.cards.cardSpecifiers.size() > 0) {
                // maybe we should allow some card filters for face down markers
                continue;
            }
            if (checkTarget(spec, markers[i-1].get())) {
                int position{-1};
                if (e.place.zone == asn::Zone::Stage) {
                    if (e.place.pos != asn::Position::NotSpecified) {
                        qWarning() << "remove marker got not NotSpecified stage pos";
                        continue;
                    }
                    co_await getStagePosition(position, e);
                }
                auto removedMarker = mPlayer->moveMarker(markerBearer, i-1, e.place, position);
                if (removedMarker) {
                    count++;
                    if (!isPayingCost()) {
                        addLastMovedCard(CardImprint(removedMarker->zone()->name(), removedMarker));
                        logMove(mPlayer, e.place.zone);
                    }
                }
            }
            if (count >= spec.number.value)
                break;
        }
        if ((spec.number.mod == asn::NumModifier::ExactMatch || spec.number.mod == asn::NumModifier::AtLeast)
                && count < spec.number.value ) {
            mPerformedInFull = false;
        }
    } else {
        qWarning() << "remove marker got not SpecificCards type";
    }
}
