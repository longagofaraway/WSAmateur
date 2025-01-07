#include "abilityPlayer.h"

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"
#include "moveCommands.pb.h"

#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"

namespace {
std::vector<uint8_t> encodeNextEffect(const asn::Effect &nextEffect) {
    std::vector<uint8_t> nextBuf;
    if (nextEffect.type == asn::EffectType::MoveCard)
        encodeMoveCard(std::get<asn::MoveCard>(nextEffect.effect), nextBuf);
    else if (nextEffect.type == asn::EffectType::ChooseCard)
        encodeChooseCard(std::get<asn::ChooseCard>(nextEffect.effect), nextBuf);
    return nextBuf;
}

bool shouldRequestConfirmation(const std::optional<asn::Effect> &nextEffect) {
    // shoud we wait for player's reaction after revealing cards?
    // if he will be choosing, then he will have enough time
    // otherwise we should wait for his reaction
    if (!nextEffect.has_value())
        return true;

    if (nextEffect->type == asn::EffectType::ChooseCard)
        return false;
    if (nextEffect->type == asn::EffectType::MoveCard) {
        const auto &effect = std::get<asn::MoveCard>(nextEffect->effect);
        if (effect.order == asn::Order::Any)
            return false;
    }
    return true;
}
}

Resumable AbilityPlayer::playLookRevealCommon(asn::EffectType type, int numCards,
                                              const std::optional<asn::Effect> &nextEffect,
                                              asn::Player zoneOwner) {
    auto deck = owner(zoneOwner)->zone("deck");

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());
    mPlayer->addExpectedCommand(CommandNextTopDeck::descriptor()->name());
    if (!mandatory())
        mPlayer->addExpectedCommand(CommandPlayEffect::descriptor()->name());
    if (nextEffect) {
        if (nextEffect->type == asn::EffectType::ChooseCard)
            mPlayer->addExpectedCommand(CommandChooseCard::descriptor()->name());
        if (nextEffect->type == asn::EffectType::MoveCard) {
            mPlayer->addExpectedCommand(CommandConfirmMove::descriptor()->name());
            const auto &moveEffect = std::get<asn::MoveCard>(nextEffect->effect);
            if (moveEffect.order == asn::Order::Any)
                mPlayer->addExpectedCommand(CommandMoveInOrder::descriptor()->name());
        }
    }

    bool confirmationRequired = false;
    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            // if we have already looked at at least 1 card, then this cancel refers to the next effect
            if (mMentionedCards.size())
                mLastCommand = cmd;
            break;
        } else if (cmd.command().Is<CommandNextTopDeck>()) {
            if (numCards == static_cast<int>(mMentionedCards.size())) {
                confirmationRequired = true;
                break;
            }

            auto card = deck->card(deck->count() - 1 - static_cast<int>(mMentionedCards.size()));
            if (!card)
                break;

            if (type == asn::EffectType::Look)
                sendLookCard(card);
            else
                sendRevealCard(card);

            if (static_cast<size_t>(deck->count()) <= mMentionedCards.size() ||
                mMentionedCards.size() == static_cast<size_t>(numCards)) {
                confirmationRequired = true;
                break;
            }
        } else if (cmd.command().Is<CommandChooseCard>() ||
                   cmd.command().Is<CommandMoveInOrder>() ||
                   cmd.command().Is<CommandConfirmMove>()) {
            mLastCommand = cmd;
            break;
        } else if (cmd.command().Is<CommandPlayEffect>()) {
            // in case of non mandatory look effect player confirms, that he is ready to move on
            break;
        }
    }

    if (confirmationRequired && shouldRequestConfirmation(nextEffect))
        co_await waitForPlayerLookConfirmation();
}

Resumable AbilityPlayer::playRevealCard(const asn::RevealCard &e,
                                        std::optional<asn::Effect> nextEffect) {
    switch (e.type) {
    case asn::RevealType::TopDeck:
        clearMentionedCards();
        mIsRevealing = true;
        if (e.number.mod == asn::NumModifier::UpTo || !mandatory()) {
            std::vector<uint8_t> buf;
            encodeRevealCard(e, buf);

            EventReveal ev;
            ev.set_effect(buf.data(), buf.size());
            if (nextEffect) {
                ev.set_next_effect_type(static_cast<int>(nextEffect->type));
                auto nextBuf = encodeNextEffect(nextEffect.value());
                ev.set_next_effect(nextBuf.data(), nextBuf.size());
            }
            mPlayer->sendToBoth(ev);

            co_await playLookRevealCommon(asn::EffectType::RevealCard, e.number.value, nextEffect);
        } else if (e.number.mod == asn::NumModifier::ExactMatch) {
            for (int i = 0; i < e.number.value; ++i) {
                auto deck = mPlayer->zone("deck");
                if (i >= deck->count())
                    break;

                auto card = deck->card(deck->count() - i - 1);
                sendRevealCard(card);
            }
            if (shouldRequestConfirmation(nextEffect))
                co_await waitForPlayerLookConfirmation();
        } else {
            assert(false);
        }
        break;
    case asn::RevealType::ChosenCards:
        setRevealChosen(true);
        break;
    case asn::RevealType::FromHand: {
        auto hand = mPlayer->zone("hand");
        for (int i = 0; i < hand->count(); ++i) {
            if (checkCard(e.card->cardSpecifiers, *hand->card(i))) {
                EventRevealFromHand event;
                event.set_hand_pos(hand->card(i)->pos());
                event.set_code(hand->card(i)->code());
                mPlayer->sendToBoth(event);
                break;
            }
        }
        break;
    }
    default:
        assert(false);
        break;
    }
}

Resumable AbilityPlayer::playLook(const asn::Look &e, std::optional<asn::Effect> nextEffect) {
    assert(e.place.zone == asn::Zone::Deck);
    clearMentionedCards();
    mIsRevealing = false;
    auto deck = owner(e.place.owner)->zone("deck");
    if (!deck->count())
        co_return;

    int numCards = e.number.value;
    if (e.valueType == asn::ValueType::Multiplier && e.multiplier) {
        if (e.multiplier->type == asn::MultiplierType::ForEach) {
            numCards *= getForEachMultiplierValue(e.multiplier.value());
        }
    }

    if (e.number.mod == asn::NumModifier::UpTo || !mandatory()) {
        std::vector<uint8_t> buf;
        encodeLook(e, buf);

        EventLook ev;
        ev.set_effect(buf.data(), buf.size());
        if (nextEffect) {
            ev.set_next_effect_type(static_cast<int>(nextEffect->type));
            auto nextBuf = encodeNextEffect(nextEffect.value());
            ev.set_next_effect(nextBuf.data(), nextBuf.size());
        }
        mPlayer->sendToBoth(ev);

        co_await playLookRevealCommon(asn::EffectType::Look, numCards, nextEffect, e.place.owner);
    } else {
        for (int i = 0; i < numCards && i < deck->count(); ++i) {
            auto card = deck->card(deck->count() - 1 - i);
            if (!card)
                break;

            sendLookCard(card);
        }
        if (shouldRequestConfirmation(nextEffect))
            co_await waitForPlayerLookConfirmation();
    }
}

void AbilityPlayer::sendLookCard(ServerCard *card) {
    EventLookTopDeck privateEvent;
    privateEvent.set_is_opponent(card->player()->id() != mPlayer->id());

    EventLookTopDeck publicEvent(privateEvent);

    privateEvent.set_card_id(card->id());
    privateEvent.set_code(card->code());
    mPlayer->sendGameEvent(privateEvent);
    mPlayer->game()->sendPublicEvent(publicEvent, mPlayer->id());

    addMentionedCard(CardImprint(card->zone()->name(), card, card->zone()->player() != mPlayer));
}

void AbilityPlayer::sendRevealCard(ServerCard *card) {
    EventRevealTopDeck event;
    event.set_card_id(card->id());
    event.set_code(card->code());
    mPlayer->sendToBoth(event);
    addMentionedCard(CardImprint("deck", card));
}
