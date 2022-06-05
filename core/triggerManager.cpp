#include "triggerManager.h"

#include <variant>

#include "abilityUtils.h"
#include "serverCardZone.h"
#include "serverPlayer.h"

namespace {
bool checkCardMatches(ServerCard *card, const asn::Target &target, ServerCard *thisCard,
                      bool &targetThisCard) {
    targetThisCard = false;
    if (target.type == asn::TargetType::ThisCard) {
        if (thisCard != card)
            return false;
        targetThisCard = true;
    } else if (target.type == asn::TargetType::SpecificCards) {
        const auto &spec = *target.targetSpecification;

        if (!checkTargetMode(spec.mode, thisCard, card))
            return false;

        if (!checkCard(spec.cards.cardSpecifiers, *card))
            return false;

        bool ownerMatched = true;
        for (const auto &spec: spec.cards.cardSpecifiers) {
            if (spec.type != asn::CardSpecifierType::Owner)
                continue;
            auto player = std::get<asn::Player>(spec.specifier);
            if (player == asn::Player::Player &&
                card->player()->id() != thisCard->player()->id())
                ownerMatched = false;
            else if (player == asn::Player::Opponent &&
                card->player()->id() == thisCard->player()->id())
                ownerMatched = false;
            break;
        }
    } else {
        assert(false);
    }
    return true;
}
bool checkCardMatches(ServerCard *card, const asn::Target &target, ServerCard *thisCard) {
    bool b;
    return checkCardMatches(card, target, thisCard, b);
}
bool checkZone(ServerCard *card, const asn::ZoneChangeTrigger& trigger,
                std::string_view from, std::string_view to, ServerCard *thisCard) {
    if (trigger.from != asn::Zone::NotSpecified && asnZoneToString(trigger.from) != from)
        return false;
    if (trigger.to != asn::Zone::NotSpecified && asnZoneToString(trigger.to) != to)
        return false;
    return true;
}
template<typename T>
std::optional<T> getTrigger(asn::TriggerType type, asn::Ability ability) {
    if (ability.type != asn::AbilityType::Auto)
        return std::nullopt;
    const auto &aa = std::get<asn::AutoAbility>(ability.ability);
    if (aa.trigger.type != type)
        return std::nullopt;
    return std::get<T>(aa.trigger.trigger);
}
}

void TriggerManager::subscribe(asn::TriggerType type, TriggerSubscriber subscriber) {
    subscribers[type].push_back(subscriber);
}

void TriggerManager::unsubscribe(asn::TriggerType type, const std::string &uniqueId) {
    if (!subscribers.contains(type))
        return;

    std::erase_if(subscribers[type], [uniqueId](const TriggerSubscriber &sub) {
        return sub.uniqueId == uniqueId;
    });
}

void TriggerManager::zoneChangeEvent(ServerCard *movedCard, std::string_view from, std::string_view to) {
    const auto &zoneChangeSubscribers = subscribers[asn::TriggerType::OnZoneChange];
    for (const auto &subscriber: zoneChangeSubscribers) {
        auto thisCard = subscriber.card.card;
        auto tOpt = getTrigger<asn::ZoneChangeTrigger>(asn::TriggerType::OnZoneChange, subscriber.ability);
        if (!tOpt.has_value())
            continue;
        auto &t = tOpt.value();
        if (!checkZone(movedCard, t, from, to, thisCard))
            continue;
        std::string_view cardZone = movedCard->zone()->name();
        bool found = false;
        for (const auto &target: t.target) {
            bool targetThisCard = false;
            if (checkCardMatches(movedCard, target, thisCard, targetThisCard)) {
                found = true;
                if (targetThisCard)
                    cardZone = to;
                break;
            }
        }
        thisCard->player()->queueDelayedAbility(subscriber.ability, thisCard, cardZone);
    }
}

void TriggerManager::phaseEvent(asn::PhaseState state, asn::Phase phase) {
    const auto &phaseSubscribers = subscribers[asn::TriggerType::OnPhaseEvent];
    for (const auto &subscriber: phaseSubscribers) {
        auto thisCard = subscriber.card.card;
        if (subscriber.ability.type != asn::AbilityType::Auto)
            continue;
        const auto &aa = std::get<asn::AutoAbility>(subscriber.ability.ability);
        if (aa.trigger.type != asn::TriggerType::OnPhaseEvent)
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
        const auto &t = std::get<asn::PhaseTrigger>(aa.trigger.trigger);
        if (t.phase != phase || (t.state != state && phase != asn::Phase::EndPhase) ||
            (t.player == asn::Player::Player && !active) ||
            (t.player == asn::Player::Opponent && active))
            continue;
        thisCard->player()->queueDelayedAbility(subscriber.ability, thisCard);
    }
}

void TriggerManager::payingCostEvent(ServerCard *target, std::optional<asn::AbilityType> type) {
    const auto &costSubscribers = subscribers[asn::TriggerType::OnPayingCost];
    for (const auto &subscriber: costSubscribers) {
        auto thisCard = subscriber.card.card;
        auto tOpt = getTrigger<asn::OnPayingCostTrigger>(asn::TriggerType::OnPayingCost, subscriber.ability);
        if (!tOpt.has_value())
            continue;
        auto &t = tOpt.value();
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
    }
}

std::string makeSubscriberId(int cardId, int abilityId) {
    return std::to_string(cardId) + std::to_string(abilityId);
}
