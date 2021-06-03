#include "serverPlayer.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "globalAbilities/globalAbilities.h"

Resumable ServerPlayer::resolveTrigger(ServerCard *card, asn::TriggerIcon trigger) {
    EventAbilityActivated event;
    auto ab = event.add_abilities();
    ab->set_zone("res");
    ab->set_type(ProtoAbilityType::ProtoClimaxTrigger);
    ab->set_cardid(0);
    ab->set_abilityid(static_cast<::google::protobuf::int32>(trigger));
    ab->set_cardcode(card->code());
    auto uniqueId = abilityHash(*ab);
    ab->set_uniqueid(uniqueId);
    sendToBoth(event);

    EventStartResolvingAbility evStart;
    evStart.set_uniqueid(uniqueId);
    sendToBoth(evStart);

    AbilityPlayer a(this);
    a.setThisCard(CardImprint("res", 0, card));
    co_await a.playAbility(triggerAbility(trigger));

    EventAbilityResolved ev2;
    ev2.set_uniqueid(uniqueId);
    sendToBoth(ev2);

    sendToBoth(EventEndResolvingAbilties());
}

void ServerPlayer::checkZoneChangeTrigger(ServerCard *movedCard, int cardId, std::string_view from, std::string_view to) {
    auto checkTrigger = [=](ServerCard *card, int id) {
        for (int j = 0; static_cast<size_t>(j) < card->abilities().size(); ++j) {
            const auto &a = card->abilities()[j];
            if (a.ability.type != asn::AbilityType::Auto)
                continue;
            const auto &aa = std::get<asn::AutoAbility>(a.ability.ability);
            if (aa.trigger.type != asn::TriggerType::OnZoneChange)
                continue;
            const auto &t = std::get<asn::ZoneChangeTrigger>(aa.trigger.trigger);
            if (t.from != asn::Zone::NotSpecified && asnZoneToString(t.from) != from)
                continue;
            if (t.to != asn::Zone::NotSpecified && asnZoneToString(t.to) != to)
                continue;
            assert(t.target.size() == 1);
            if (t.target[0].type == asn::TargetType::ThisCard) {
                if (card != movedCard)
                    continue;
            } else if (t.target[0].type != asn::TargetType::SpecificCards) {
                const auto &spec = *t.target[0].targetSpecification;
                if (!checkCard(spec.cards.cardSpecifiers, *movedCard))
                    continue;
            }

            TriggeredAbility ta;
            ta.card = CardImprint(card->zone()->name(), id, card);
            ta.type = ProtoCard;
            ta.abilityId = j;
            mQueue.push_back(ta);
        }
    };

    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;
        checkTrigger(card, i);
    }

    if (movedCard->zone()->name() != "stage")
        checkTrigger(movedCard, cardId);
}

void ServerPlayer::checkGlobalEncore(ServerCard *movedCard, int cardId, std::string_view from, std::string_view to) {
    if (from == "stage" && to == "wr" && movedCard->zone()->name() == "wr") {
        TriggeredAbility ta;
        ta.card = CardImprint(movedCard->zone()->name(), cardId, movedCard);
        ta.type = ProtoGlobal;
        ta.abilityId = static_cast<int>(GlobalAbility::Encore);
        mQueue.push_back(ta);
    }
}

void ServerPlayer::checkOnAttack(ServerCard *attCard) {
    auto &abs = attCard->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnAttack)
            continue;
        const auto &trig = std::get<asn::OnAttackTrigger>(autoab.trigger.trigger);
        if (trig.target.type != asn::TargetType::ThisCard)
            continue;
        TriggeredAbility a;
        a.card = CardImprint(attCard->zone()->name(), attCard->pos(), attCard);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

void ServerPlayer::checkPhaseTrigger(asn::PhaseState state, asn::Phase phase) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        auto &abs = card->abilities();
        for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
            if (abs[i].ability.type != asn::AbilityType::Auto)
                continue;
            const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
            if (autoab.trigger.type != asn::TriggerType::OnPhaseEvent)
                continue;
            const auto &trig = std::get<asn::PhaseTrigger>(autoab.trigger.trigger);
            if (trig.phase != phase || trig.state != state ||
                (trig.player == asn::Player::Player && !mActive) ||
                (trig.player == asn::Player::Opponent && mActive))
                continue;

            TriggeredAbility a;
            a.card = CardImprint(card->zone()->name(), card->pos(), card);
            a.type = ProtoCard;
            a.abilityId = i;
            mQueue.push_back(a);
        }
    }
}

void ServerPlayer::checkOnBackup(ServerCard *card) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnBackupOfThis)
            continue;

        TriggeredAbility a;
        a.card = CardImprint(card->zone()->name(), card->pos(), card);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

void ServerPlayer::checkOnTriggerReveal(ServerCard *revealedCard) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        auto &abs = card->abilities();
        for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
            if (abs[i].ability.type != asn::AbilityType::Auto)
                continue;
            const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
            if (autoab.trigger.type != asn::TriggerType::OnTriggerReveal)
                continue;
            const auto &trig = std::get<asn::TriggerRevealTrigger>(autoab.trigger.trigger);
            if (!checkCard(trig.card.cardSpecifiers, *revealedCard))
                continue;

            TriggeredAbility a;
            a.card = CardImprint(card->zone()->name(), card->pos(), card);
            a.type = ProtoCard;
            a.abilityId = i;
            mQueue.push_back(a);
        }
    }
}

void ServerPlayer::checkOtherTrigger(const std::string &code) {
    auto stage = zone("stage");
    for (int i = 0; i < stage->count(); ++i) {
        auto card = stage->card(i);
        if (!card)
            continue;

        auto &abs = card->abilities();
        for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
            if (abs[i].ability.type != asn::AbilityType::Auto)
                continue;

            const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
            if (autoab.trigger.type != asn::TriggerType::OtherTrigger)
                continue;

            const auto &trig = std::get<asn::OtherTrigger>(autoab.trigger.trigger);
            if (code != trig.cardCode)
                continue;

            TriggeredAbility a;
            a.card = CardImprint(card->zone()->name(), card->pos(), card);
            a.type = ProtoCard;
            a.abilityId = i;
            mQueue.push_back(a);
        }
    }
}

void ServerPlayer::triggerBackupAbility(ServerCard *card) {
    if (!card->isCounter())
        return;

    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Act)
            continue;
        const auto &actab = std::get<asn::ActAbility>(abs[i].ability.ability);
        if (actab.keywords.empty() || actab.keywords[0] != asn::Keyword::Backup)
            continue;

        TriggeredAbility a;
        a.card = CardImprint(card->zone()->name(), card->pos(), card);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);

        return;
    }
}

void ServerPlayer::checkOnReversed(ServerCard *card) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnReversed)
            continue;
        TriggeredAbility a;
        a.card = CardImprint(card->zone()->name(), card->pos(), card);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}

void ServerPlayer::checkOnBattleOpponentReversed(ServerCard *attCard, ServerCard *battleOpp) {
    auto &abs = attCard->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Auto)
            continue;
        const auto &autoab = std::get<asn::AutoAbility>(abs[i].ability.ability);
        if (autoab.trigger.type != asn::TriggerType::OnBattleOpponentReversed)
            continue;
        const auto &trig = std::get<asn::BattleOpponentReversedTrigger>(autoab.trigger.trigger);
        if (!checkCard(trig.card.cardSpecifiers, *battleOpp))
            continue;
        TriggeredAbility a;
        a.card = CardImprint(attCard->zone()->name(), attCard->pos(), attCard);
        a.type = ProtoCard;
        a.abilityId = i;
        mQueue.push_back(a);
    }
}