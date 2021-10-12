#include "abilityPlayer.h"

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"
#include "moveCommands.pb.h"

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
    player->addExpectedCommand(CommandChoice::descriptor()->name());

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
    auto pzone = player->zone(asnZoneToString(e.from.zone));
    const auto &spec = *e.target.targetSpecification;

    if (!isPayingCost())
        clearLastMovedCards();

    for (int i = 0; i < spec.number.value; ++i) {
        auto card = pzone->topCard();
        if (!card)
            break;

        player->moveCard(asnZoneToString(e.from.zone), pzone->count() - 1, asnZoneToString(e.to[toZoneIndex].zone), toIndex, revealChosen());
        if (!isPayingCost())
            addLastMovedCard(CardImprint(card->zone()->name(), card, e.to[toZoneIndex].owner == asn::Player::Opponent));

        if (e.from.zone == asn::Zone::Deck || e.to[toZoneIndex].zone == asn::Zone::Clock)
            co_await player->checkRefreshAndLevelUp();
    }
}

Resumable AbilityPlayer::playMoveCard(const asn::MoveCard &e) {
    if ((e.target.type == asn::TargetType::ChosenCards && chosenCards().empty()) ||
        ((e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) &&
            mentionedCards().empty()) ||
        (e.target.type == asn::TargetType::ThisCard && thisCard().zone != thisCard().card->zone()->name()))
        co_return;

    if ((e.from.pos == asn::Position::Top || e.from.pos == asn::Position::Bottom)) {
        auto player = owner(e.from.owner);
        if (player->zone(asnZoneToString(e.from.zone))->count() == 0)
            co_return;
    }

    if (e.target.type == asn::TargetType::BattleOpponent) {
        if (thisCard().card->zone()->name() != "stage")
            co_return;

        auto card = mPlayer->oppositeCard(thisCard().card);
        if (!card)
            co_return;
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
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(mandatory());
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
                co_return;
            }
        } else if (e.to[toZoneIndex].pos == asn::Position::SlotThisWasIn) {
            positionSet = true;
            toIndex = thisCard().card->prevStagePos();
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
            cardsToMove[card.card->pos()] = card.card;
        }
    } else if (e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) {
        for (const auto &card: mentionedCards()) {
            player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            cardsToMove[card.card->pos()] = card.card;
        }
    } else if (e.target.type == asn::TargetType::SpecificCards) {
        // can't process top cards of deck in the main cycle below,
        // because we don't know ids of next cards in case of refresh
        if (e.from.pos == asn::Position::Top) {
            co_await moveTopDeck(e, toZoneIndex, toIndex);
            co_return;
        }
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
        mTriggerIcon && mTriggerIcon.value() == asn::TriggerIcon::Standby)
        owner(asn::Player::Opponent)->triggerOnOppCharPlacedByStandby();
    for (auto it = cardsToMove.rbegin(); it != cardsToMove.rend(); ++it) {
        player->moveCard(it->second->zone()->name(), it->first, asnZoneToString(e.to[toZoneIndex].zone), toIndex, revealChosen());
        if (!isPayingCost())
            addLastMovedCard(CardImprint(it->second->zone()->name(), it->second, e.to[toZoneIndex].owner == asn::Player::Opponent));

        if (e.to[toZoneIndex].pos == asn::Position::SlotThisWasIn)
            player->setCardState(it->second, asn::State::Rested);

        // TODO: refresh and levelup are triggered at the same time, give choice
        if (e.from.zone == asn::Zone::Deck || e.to[toZoneIndex].zone == asn::Zone::Clock)
            co_await player->checkRefreshAndLevelUp();
    }
}
