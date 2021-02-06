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
            for (int i = chooseCmd.ids_size() - 1; i >= 0; --i)
                mContext.chosenCards.push_back(CardImprint(chooseCmd.zone(), chooseCmd.ids(i), chooseCmd.owner() == ProtoOwner::ProtoOpponent));
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

Resumable ServerPlayer::playEffect(const asn::Effect &e) {
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
    default:
        assert(false);
        break;
    }
}

Resumable ServerPlayer::playEventAbility(const asn::EventAbility &a) {
    for (const auto &effect: a.effects)
        co_await playEffect(effect);
}

Resumable ServerPlayer::playAbility(const asn::Ability &a) {
    switch(a.type) {
    case asn::AbilityType::Event:
        co_await playEventAbility(std::get<asn::EventAbility>(a.ability));
        break;
    }
}

