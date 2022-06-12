#pragma once

#include <unordered_map>
#include <vector>

#include "abilities.h"
#include "cardImprint.h"

class ServerPlayer;

struct TriggerSubscriber {
    std::string uniqueId;
    asn::Ability ability;
    CardImprint card;
};

using Subscribers = std::unordered_map<asn::TriggerType,
                                       std::vector<TriggerSubscriber>>;

class TriggerManager {
    Subscribers subscribers_;

public:
    void subscribe(asn::TriggerType type, TriggerSubscriber subscriber);
    void unsubscribe(asn::TriggerType type, const std::string &uniqueId);
    void zoneChangeEvent(ServerCard *movedCard, std::string_view from, std::string_view to);
    void phaseEvent(asn::PhaseState state, asn::Phase phase);
    void payingCostEvent(ServerCard *target, std::optional<asn::AbilityType> type = std::nullopt);
    void damageCancelEvent(ServerCard *attCard, bool cancelled);
};

std::string makeSubscriberId(int cardId, int abilityId);
