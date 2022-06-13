#include "triggerManager.h"

#include <variant>

#include "abilityUtils.h"
#include "serverCardZone.h"
#include "serverPlayer.h"

namespace {
bool checkZone(ServerCard *card, const asn::ZoneChangeTrigger& trigger,
                std::string_view from, std::string_view to, ServerCard *thisCard) {
    if (trigger.from != asn::Zone::NotSpecified && asnZoneToString(trigger.from) != from)
        return false;
    if (trigger.to != asn::Zone::NotSpecified && asnZoneToString(trigger.to) != to)
        return false;
    return true;
}
}

void TriggerManager::subscribe(asn::TriggerType type, TriggerSubscriber subscriber) {
    subscribers_[type].push_back(subscriber);
}

void TriggerManager::unsubscribe(asn::TriggerType type, const std::string &uniqueId) {
    if (!subscribers_.contains(type))
        return;

    std::erase_if(subscribers_[type], [uniqueId](const TriggerSubscriber &sub) {
        return sub.uniqueId == uniqueId;
    });
}

void TriggerManager::zoneChangeEvent(ServerCard *movedCard, std::string_view from, std::string_view to) {
    const auto &zoneChangeSubscribers = subscribers_[asn::TriggerType::OnZoneChange];
    for (const auto &subscriber: zoneChangeSubscribers) {
        auto thisCard = subscriber.card.card;
        if (subscriber.ability.type != asn::AbilityType::Auto)
            continue;
        const auto &aa = std::get<asn::AutoAbility>(subscriber.ability.ability);
        for (const auto &trigger: aa.triggers) {
            if (trigger.type != asn::TriggerType::OnZoneChange)
                continue;
            const auto &t = std::get<asn::ZoneChangeTrigger>(trigger.trigger);
            if (!checkZone(movedCard, t, from, to, thisCard))
                continue;
            std::string_view cardZone = movedCard->zone()->name();
            bool found = false;
            for (const auto &target: t.target) {
                if (checkCardMatches(movedCard, target, thisCard)) {
                    found = true;
                    if (target.type == asn::TargetType::ThisCard && thisCard == movedCard)
                        cardZone = to;
                    break;
                }
            }
            thisCard->player()->queueDelayedAbility(subscriber.ability, thisCard, cardZone);
            break;
        }
    }
}

void TriggerManager::phaseEvent(asn::PhaseState state, asn::Phase phase) {
    const auto &phaseSubscribers = subscribers_[asn::TriggerType::OnPhaseEvent];
    for (const auto &subscriber: phaseSubscribers) {
        auto thisCard = subscriber.card.card;
        if (subscriber.ability.type != asn::AbilityType::Auto)
            continue;
        const auto &aa = std::get<asn::AutoAbility>(subscriber.ability.ability);
        for (const auto &trigger: aa.triggers) {
            if (trigger.type != asn::TriggerType::OnPhaseEvent)
                continue;
            // do not activate alarm if the card is on the stage
            // (otherwise ability is shown for a brief moment, but the condition is not met)
            // also prevent activating non alarm abilities from top clock
            bool alarm = std::any_of(aa.keywords.begin(), aa.keywords.end(),
                            [](asn::Keyword k){ return k == asn::Keyword::Alarm; });
            bool topClock = (thisCard->zone()->name() == "clock") &&
                            (thisCard->pos() + 1 == thisCard->zone()->count());
            if (alarm != topClock)
                continue;

            bool active = thisCard->player()->active();
            const auto &t = std::get<asn::PhaseTrigger>(trigger.trigger);
            if (t.phase != phase || (t.state != state && phase != asn::Phase::EndPhase) ||
                (t.player == asn::Player::Player && !active) ||
                (t.player == asn::Player::Opponent && active))
                continue;
            thisCard->player()->queueDelayedAbility(subscriber.ability, thisCard);
            break;
        }
    }
}

void TriggerManager::payingCostEvent(ServerCard *target, std::optional<asn::AbilityType> type) {
    const auto &costSubscribers = subscribers_[asn::TriggerType::OnPayingCost];
    for (const auto &subscriber: costSubscribers) {
        auto thisCard = subscriber.card.card;
        if (subscriber.ability.type != asn::AbilityType::Auto)
            continue;
        const auto &aa = std::get<asn::AutoAbility>(subscriber.ability.ability);
        for (const auto &trigger: aa.triggers) {
            if (trigger.type != asn::TriggerType::OnPayingCost)
                continue;
            const auto &t = std::get<asn::OnPayingCostTrigger>(trigger.trigger);
            if (target->player()->id() != thisCard->player()->id())
                continue;
            if (t.target.type != asn::TargetType::SpecificCards)
                assert(false);
            if (!checkCardMatches(target, t.target, thisCard))
                continue;
            if (type.has_value()) {
                if (type.value() != t.abilityType)
                    continue;
                if (thisCard->type() != asn::CardType::Char)
                    continue;
            }
            thisCard->player()->queueDelayedAbility(subscriber.ability, thisCard, "", true);
            break;
        }
    }
}

void TriggerManager::damageCancelEvent(ServerCard *attCard, bool cancelled) {
    const auto &damageCancelSubscribers = subscribers_[asn::TriggerType::OnDamageCancel];
    for (const auto &subscriber: damageCancelSubscribers) {
        auto thisCard = subscriber.card.card;
        if (subscriber.ability.type != asn::AbilityType::Auto)
            continue;
        const auto &aa = std::get<asn::AutoAbility>(subscriber.ability.ability);
        for (const auto &trigger: aa.triggers) {
            if (trigger.type != asn::TriggerType::OnDamageCancel)
                continue;
            const auto &t = std::get<asn::OnDamageCancelTrigger>(trigger.trigger);
            if (t.cancelled != cancelled)
                continue;
            if (!checkCardMatches(attCard, t.damageDealer, thisCard))
                continue;
            thisCard->player()->queueDelayedAbility(subscriber.ability, thisCard, "");
            break;
        }
    }
}

std::string makeSubscriberId(int cardId, int abilityId) {
    return std::to_string(cardId) + std::to_string(abilityId);
}
