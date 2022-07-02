#include "abilityPlayer.h"

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"

#include "codecs/encode.h"
#include "serverPlayer.h"

namespace {
int numOfDifferentlyNamedEvents(ServerPlayer *player) {
    auto zone = player->zone(asn::Zone::Memory);
    std::set<std::string> names;
    for (int i = 0; i < zone->count(); ++i) {
        auto card = zone->card(i);
        names.insert(card->name());
    }
    return static_cast<int>(names.size());
}

std::vector<ServerCard*> getChosenCards(const GameCommand &cmd, ServerCardZone *zone, int &sum) {
    sum = 0;
    CommandChooseCard chooseCmd;
    cmd.command().UnpackTo(&chooseCmd);
    std::vector<ServerCard*> cards;
    for (int i = chooseCmd.positions_size() - 1; i >= 0; --i) {
        auto card = zone->card(chooseCmd.positions(i));
        if (!card)
            continue;
        if (card->type() != asn::CardType::Char)
            continue;
        sum += card->level();
        cards.push_back(card);
    }
    return cards;
}
}

Resumable AbilityPlayer::playW87_53() {
    clearMentionedCards();
    auto deck = mPlayer->getOpponent()->zone("deck");
    if (!deck->count())
        co_return;

    const int numCards = 2;
    asn::Look look;
    look.number = asn::Number{asn::NumModifier::UpTo, numCards};
    look.valueType = asn::ValueType::Raw;
    look.place = asn::Place{asn::Position::Top, asn::Zone::Deck, asn::Player::Player};
    std::vector<uint8_t> buf;
    encodeLook(look, buf);

    asn::Card card;
    card.cardSpecifiers.push_back(asn::CardSpecifier{asn::CardSpecifierType::CardType, asn::CardType::Char});
    asn::TargetSpecificCards spec;
    spec.mode = asn::TargetMode::Any;
    spec.number = asn::Number{asn::NumModifier::UpTo, numCards};
    spec.cards = card;
    asn::Target target;
    target.type = asn::TargetType::SpecificCards;
    target.targetSpecification = spec;
    asn::TargetAndPlace tp;
    tp.target = target;
    tp.placeType = asn::PlaceType::Selection;
    asn::ChooseCard choose;
    choose.executor = asn::Player::Player;
    choose.targets.push_back(tp);
    std::vector<uint8_t> nextBuf;
    encodeChooseCard(choose, nextBuf);

    EventLook ev;
    ev.set_effect(buf.data(), buf.size());
    ev.set_next_effect_type(static_cast<int>(asn::EffectType::ChooseCard));
    ev.set_next_effect(nextBuf.data(), nextBuf.size());

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());
    mPlayer->addExpectedCommand(CommandNextTopDeck::descriptor()->name());
    mPlayer->addExpectedCommand(CommandChooseCard::descriptor()->name());

    bool repeatSendEffect = false;
    std::optional<GameCommand> chooseCmd;
    while (true) {
        if (repeatSendEffect) {
            mPlayer->sendToBoth(ev);
            repeatSendEffect = false;
        }
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            co_return;
        } else if (cmd.command().Is<CommandNextTopDeck>()) {
            if (numCards == static_cast<int>(mMentionedCards.size()))
                break;

            auto card = deck->card(deck->count() - 1 - static_cast<int>(mMentionedCards.size()));
            if (!card)
                break;

            sendLookCard(card);

            if (static_cast<size_t>(deck->count()) <= mMentionedCards.size() ||
                mMentionedCards.size() == static_cast<size_t>(numCards))
                break;
        } else if (cmd.command().Is<CommandChooseCard>()) {
            int sum = 0;
            getChosenCards(cmd, deck, sum);
            if (sum > numOfDifferentlyNamedEvents(mPlayer)) {
                repeatSendEffect = true;
                continue;
            }
            chooseCmd = cmd;
            break;
        }
    }

    clearChosenCards();

    EventChooseCard chooseEvent;
    chooseEvent.set_effect(buf.data(), buf.size());
    chooseEvent.set_mandatory(mandatory());
    auto targets = getTargets(target, tp.placeType, tp.place);
    for (const auto &t: targets) {
        auto card = chooseEvent.add_cards();
        card->set_id(t->id());
        card->set_position(t->pos());
    }

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandChooseCard::descriptor()->name());
    mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());

    while (true) {
        mPlayer->sendToBoth(chooseEvent);
        GameCommand cmd;
        if (chooseCmd) {
            cmd = *chooseCmd;
            chooseCmd.reset();
        } else {
            cmd = co_await waitForCommand();
        }
        if (cmd.command().Is<CommandCancelEffect>()) {
            setCanceled(true);
            break;
        } else if (cmd.command().Is<CommandChooseCard>()) {
            int sum = 0;
            auto cards = getChosenCards(cmd, deck, sum);
            if (sum > numOfDifferentlyNamedEvents(mPlayer)) {
                continue;
            }
            for (const auto card: cards) {
                addChosenCard(CardImprint(card->zone()->name(), card, true));
                removeMentionedCard(card->id());
            }
            break;
        }
    }
}
