#include <array>

#include <unordered_set>

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"
#include "globalAbilities/globalAbilities.h"

bool ServerPlayer::canBePlayed(ServerCard *thisCard, const asn::Ability &a) {
    AbilityPlayer aplayer(this);
    aplayer.setThisCard(thisCard);
    return aplayer.canBePlayed(a);
}

void ServerPlayer::resolveAllContAbilities() {
    for (const auto &zoneIt: zones()) {
        const auto zone = zoneIt.second.get();
        for (int i = 0; i < zone->count(); ++i) {
            auto card = zone->card(i);
            if (!card)
                continue;

            playContAbilities(card);
        }
    }
}

namespace {
bool checkTriggerIconGainValidity(const asn::ContAbility &a, const std::string &name) {
    const auto &effect = std::get<asn::TriggerIconGain>(a.effects[0].effect);
    if (effect.target.type != asn::TargetType::ThisCard && name != "stage")
        return false;

    return true;
}

bool isEffectPlayedFromHand(const asn::EffectType &type) {
    static std::array<asn::EffectType, 2> handEffects{
        asn::EffectType::EarlyPlay,
        asn::EffectType::CannotPlay
    };
    return std::any_of(handEffects.begin(), handEffects.end(),
                       [type](asn::EffectType t){
        return type == t;
    });
}

bool checkColorReqEffectValidity(const asn::ContAbility &a, const std::string &zone) {
    const auto &effect = std::get<asn::CanPlayWithoutColorRequirement>(a.effects[0].effect);
    if (effect.target.type == asn::TargetType::ThisCard && zone != "hand")
        return false;
    if (effect.target.type != asn::TargetType::ThisCard && zone != "stage")
        return false;
    return true;
}

bool checkAbilityValidForZone(const asn::ContAbility &a, const std::string &zoneName) {
    if (a.effects.empty())
        return false;
    const auto effectType = a.effects[0].type;

    static std::unordered_set<asn::EffectType> specialHandlingEffects{
        asn::EffectType::EarlyPlay,
        asn::EffectType::CannotPlay,
        asn::EffectType::TriggerIconGain,
        asn::EffectType::CanPlayWithoutColorRequirement
    };
    for (size_t i = 1; i < a.effects.size(); ++i)
        assert(!specialHandlingEffects.contains(a.effects[i].type));

    if (effectType == asn::EffectType::TriggerIconGain)
        return checkTriggerIconGainValidity(a, zoneName);
    if (effectType == asn::EffectType::CanPlayWithoutColorRequirement)
        return checkColorReqEffectValidity(a, zoneName);
    if (isEffectPlayedFromHand(effectType))
        return zoneName == "hand";
    return zoneName == "stage" || zoneName == "climax";
}
}

void ServerPlayer::playContAbilities(ServerCard *card, bool revert) {
    auto &abs = card->abilities();
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Cont)
            continue;
        const auto &cont = std::get<asn::ContAbility>(abs[i].ability.ability);

        if (!checkAbilityValidForZone(cont, card->zone()->name()))
            continue;

        if (revert && !abs[i].active)
            continue;

        AbilityPlayer a(this);
        a.setThisCard(card);
        a.setAbilityId(abs[i].id);
        if (!revert) {
            a.playContAbility(cont, abs[i].active);
        } else {
            a.revertContAbility(cont);
            abs[i].active = false;
        }
    }
}

Resumable ServerPlayer::playEventEffects(ServerCard *card) {
    auto expectedCopy = mExpectedCommands;
    clearExpectedComands();

    auto &abs = card->abilities();
    bool needEndEvent = false;
    for (int i = 0; i < static_cast<int>(abs.size()); ++i) {
        if (abs[i].ability.type != asn::AbilityType::Event)
            continue;

        needEndEvent = true;
        EventAbilityActivated event;
        auto eventAbility = event.add_abilities();
        eventAbility->set_zone(card->zone()->name());
        eventAbility->set_type(ProtoCard);
        eventAbility->set_card_id(card->id());
        eventAbility->set_ability_id(i);
        eventAbility->set_card_code(card->code());
        auto uniqueId = abilityHash(*eventAbility);
        eventAbility->set_unique_id(uniqueId);
        sendToBoth(event);

        EventStartResolvingAbility evStart;
        evStart.set_unique_id(uniqueId);
        sendToBoth(evStart);

        AbilityPlayer a(this);
        a.setThisCard(card);
        co_await a.playAbility(abs[i].ability);

        EventAbilityResolved evEnd;
        evEnd.set_unique_id(uniqueId);
        sendToBoth(evEnd);
    }

    if (needEndEvent)
        sendToBoth(EventEndResolvingAbilties());

    mExpectedCommands = expectedCopy;
}

Resumable ServerPlayer::limitedCheckTiming() {
    if (mHelperQueue.empty())
        co_return;

    auto expectedCopy = mExpectedCommands;
    clearExpectedComands();

    bool needEndEvent = !mHelperQueue.empty();
    while (mHelperQueue.size()) {
        sendActivatedAbilitiesToClient(true);
        std::optional<uint32_t> uniqueId;
        co_await getAbilityToPlay(uniqueId, true);
        if (!uniqueId.has_value())
            continue;

        co_await playChosenAbility(uniqueId.value(), true);
    }
    if (needEndEvent)
        sendToBoth(EventEndResolvingAbilties());

    mExpectedCommands = expectedCopy;
}


void ServerPlayer::sendActivatedAbilitiesToClient(bool helperQueue) {
    auto &queue = helperQueue ? mHelperQueue : mQueue;
    EventAbilityActivated event;
    for (size_t i = 0; i < queue.size(); ++i) {
        if (queue[i].uniqueId != 0)
            continue;
        auto ab = event.add_abilities();
        ab->set_zone(queue[i].card.zone);
        ab->set_type(queue[i].type);
        ab->set_card_id(queue[i].card.card->id());
        ab->set_ability_id(queue[i].abilityId);
        ab->set_card_code(queue[i].card.card->code());
        if (queue[i].ability) {
            auto enc = encodeAbility(*queue[i].ability);
            ab->set_ability(enc.data(), enc.size());
        }
        queue[i].uniqueId = abilityHash(*ab);
        ab->set_unique_id(queue[i].uniqueId);
    }
    if (event.abilities_size())
        sendToBoth(event);
}

Resumable ServerPlayer::getAbilityToPlay(std::optional<uint32_t> &uniqueId, bool helperQueue) {
    auto &queue = helperQueue ? mHelperQueue : mQueue;

    int playableCount = 0;
    for (size_t i = 0; i < queue.size(); ++i) {
        if (canBePlayed(queue[i].card.card, queue[i].getAbility())) {
            uniqueId = queue[i].uniqueId;
            playableCount++;
        }
    }

    if (playableCount > 1) {
        clearExpectedComands();
        addExpectedCommand(CommandPlayAbility::descriptor()->name());
        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandPlayAbility>()) {
                CommandPlayAbility choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                uniqueId = choiceCmd.unique_id();
                break;
            }
        }
    } else if (playableCount == 0) {
        // Only abilities that cannot be played or cannot be paid for are left
        while(queue.size()) {
            EventAbilityResolved ev2;
            ev2.set_unique_id(queue[0].uniqueId);
            sendToBoth(ev2);
            queue.erase(queue.begin());
        }
    }
}

Resumable ServerPlayer::playChosenAbility(uint32_t uniqueId, bool helperQueue) {
    auto &queue = helperQueue ? mHelperQueue : mQueue;
    for (size_t j = 0; j < queue.size(); ++j) {
        if (queue[j].uniqueId == uniqueId) {
            EventStartResolvingAbility evStart;
            evStart.set_unique_id(uniqueId);
            sendToBoth(evStart);

            AbilityPlayer a(this);
            a.setThisCard(queue[j].card);
            a.setCardFromTrigger(queue[j].cardFromTrigger);
            co_await a.playAbility(queue[j].getAbility());

            EventAbilityResolved evEnd;
            evEnd.set_unique_id(uniqueId);
            sendToBoth(evEnd);
            queue.erase(queue.begin() + j);

            break;
        }
    }
}

Resumable ServerPlayer::checkTiming() {
    // protocol for playing abilities
    // 0. Resolve Rule Actions and resolve all abilities of turn player, if it is not this player's turn.
    // 1. Server sends EventAbilityActivated with activated abilities. Client shows them.
    // 2. If there are more than 1 playable ability, client must send, which one he wants to play
    //    using CommandPlayAbility. Otherwise, server will automatically choose to play the single playable ability.
    // 3. Server sends EventStartResolvingAbility to indicate, which ability it is going to play.
    //    Client marks that ability as active.
    // 4. Server sends EventAbilityResolved after the ability is resolved.
    // 5. At this point more abilities may be triggered. If this is not the master of the turn and
    //    the master of the turn has abilities to resolve, send EventEndResolvingAbilties as this player,
    //    and perform checkTiming as active player. If new abilities appeared during check timing,
    //    proceed to point 1 but send EventAbilityActivated only with new abilities. If there are no new abilities,
    //    but not all abilities are resolved, proceed to point 2.
    // 6. After all abilities are resolved, server sends EventEndResolvingAbilties.
    //    Client uses this signal to restore ui state after playing abilities.
    if (mQueue.empty())
        co_return;

    auto expectedCopy = mExpectedCommands;
    clearExpectedComands();
    while (mQueue.size()) {
        // first play all rule actions
        co_await mGame->processRuleActions();

        if (!mActive && getOpponent()->hasActivatedAbilities()) {
            // priority to the master of the turn
            for (size_t i = 0; i < mQueue.size(); ++i)
                mQueue[i].uniqueId = 0;
            break;
        }

        sendActivatedAbilitiesToClient();
        std::optional<uint32_t> uniqueId;
        co_await getAbilityToPlay(uniqueId);
        if (!uniqueId.has_value())
            continue;

        co_await playChosenAbility(uniqueId.value());
    }
    sendToBoth(EventEndResolvingAbilties());

    mExpectedCommands = expectedCopy;
}

Resumable ServerPlayer::processRuleActions() {
    if (mQueue.empty())
        co_return;

    bool actionFound = false;
    do {
        actionFound = false;
        for (size_t i = 0; i < mQueue.size(); ++i) {
            if (mQueue[i].type != ProtoAbilityType::ProtoRuleAction)
                continue;
            AbilityPlayer a(this);
            a.setThisCard(mQueue[i].card);
            co_await a.playAbility(ruleActionAbility(static_cast<RuleAction>(mQueue[i].abilityId)));
            mQueue.erase(mQueue.begin() + i);
            actionFound = true;
        }
    } while (actionFound);
}

bool ServerPlayer::hasActivatedAbilities() const {
    return !mQueue.empty();
}

bool ServerPlayer::hasTriggeredRuleActions() const {
    return std::any_of(mQueue.begin(), mQueue.end(), [](const auto &elem) {
        return elem.type == ProtoRuleAction;
    });
}

