#include "abilities.pb.h"

#include "serverPlayer.h"
#include "codecs/encode.h"

namespace {
std::string_view asnZoneToString(asn::Zone zone) {
    switch (zone) {
    case asn::Zone::Climax:
        return "climax";
    case asn::Zone::Clock:
        return "clock";
    case asn::Zone::Deck:
        return "deck";
    case asn::Zone::Hand:
        return "hand";
    case asn::Zone::Level:
        return "level";
    case asn::Zone::Memory:
        return "memory";
    case asn::Zone::Stage:
        return "stage";
    case asn::Zone::Stock:
        return "stock";
    case asn::Zone::WaitingRoom:
        return "wr";
    default:
        assert(false);
        return "";
    }
}
}

Resumable ServerPlayer::playNonMandatory(const asn::NonMandatory &e) {
    mContext.mandatory = false;
    for (const auto &effect: e.effect)
        co_await playEffect(effect);
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
    if (!mContext.mandatory)
        addExpectedCommand(CommandIgnoreEffect::GetDescriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandIgnoreEffect>()) {
            co_return;
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
            //add checks
            for (int i = 0; i < chooseCmd.ids_size(); ++i)
                mContext.chosenCards.push_back(ChosenCard(chooseCmd.zone(), chooseCmd.ids(i)));
            break;
        }
    }
}

void ServerPlayer::playMoveCard(const asn::MoveCard &e) {
    assert(e.executor == asn::Player::Player);
    assert(e.to.size() == 1);
    assert(e.to[0].zone != asn::Zone::Stage);
    if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: mContext.chosenCards) {
            moveCard(card.zone, card.id, asnZoneToString(e.to[0].zone));
        }
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
        playMoveCard(std::get<asn::MoveCard>(e.effect));
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
    mContext = AbilityContext();

    switch(a.type) {
    case asn::AbilityType::Event:
        co_await playEventAbility(std::get<asn::EventAbility>(a.ability));
        break;
    }
}

