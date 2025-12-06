#include "triggerInit.h"
#include "trigger.h"

#include "ability_maker_gen.h"
#include "language_parser.h"


decltype(asn::Trigger::trigger) getDefaultTrigger(asn::TriggerType type) {
    auto thisCardTarget = asn::Target();
    thisCardTarget.type = asn::TargetType::ThisCard;

    decltype(asn::Trigger::trigger) trigger;
    switch (type) {
    case asn::TriggerType::OnZoneChange: {
        auto tr = asn::ZoneChangeTrigger();
        tr.target.push_back(thisCardTarget);
        tr.from = asn::Zone::Hand;
        tr.to = asn::Zone::Stage;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnPlay: {
        auto tr = asn::OnPlayTrigger();
        tr.target = thisCardTarget;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnStateChange: {
        auto tr = asn::StateChangeTrigger();
        tr.target = thisCardTarget;
        tr.state = asn::State::Reversed;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnAttack: {
        auto tr = asn::OnAttackTrigger();
        tr.target = thisCardTarget;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnBackupOfThis: {
        trigger = std::monostate();
        break;
    }
    case asn::TriggerType::OnTriggerReveal: {
        auto tr = asn::TriggerRevealTrigger();
        asn::CardSpecifier spec{
            .type = asn::CardSpecifierType::CardType,
            .specifier = asn::CardType::Climax
        };
        tr.card.cardSpecifiers.push_back(spec);
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnPhaseEvent: {
        auto tr = asn::PhaseTrigger();
        tr.phase = asn::Phase::ClimaxPhase;
        tr.player = asn::Player::Player;
        tr.state = asn::PhaseState::Start;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnEndOfThisCardsAttack: {
        trigger = std::monostate();
        break;
    }
    case asn::TriggerType::OnOppCharPlacedByStandbyTriggerReveal: {
        trigger = std::monostate();
        break;
    }
    case asn::TriggerType::OnBeingAttacked: {
        auto tr = asn::OnBeingAttackedTrigger();
        tr.target = thisCardTarget;
        tr.attackType = asn::AttackType::Frontal;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnDamageCancel: {
        auto tr = asn::OnDamageCancelTrigger();
        tr.damageDealer = thisCardTarget;
        tr.cancelled = true;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnDamageTakenCancel: {
        auto tr = asn::OnDamageTakenCancelTrigger();
        tr.cancelled = false;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnPayingCost: {
        auto tr = asn::OnPayingCostTrigger();
        tr.abilityType = asn::AbilityType::Auto;
        tr.target = thisCardTarget;
        trigger = tr;
        break;
    }
    case asn::TriggerType::OnActAbillity: {
        auto tr = asn::OnActAbillityTrigger();
        tr.player = asn::Player::Player;
        trigger = tr;
        break;
    }
    }
    return trigger;
}

std::optional<asn::Trigger> getTriggerFromPreset(QString preset) {
    auto thisTarget = asn::Target();
    thisTarget.type = asn::TargetType::ThisCard;
    auto defaultNumber = asn::Number{.mod=asn::NumModifier::AtLeast,
        .value=1};

    auto emptyTargetSpec = asn::TargetSpecificCards{
            .mode=asn::TargetMode::Any,
            .number=defaultNumber
    };

    if (preset == "ZoneChangeHandStage") {
        auto trigger = asn::Trigger{};
        auto tr = asn::ZoneChangeTrigger();
        tr.target.push_back(thisTarget);
        tr.from = asn::Zone::Hand;
        tr.to = asn::Zone::Stage;
        trigger.trigger = tr;
        trigger.type = asn::TriggerType::OnZoneChange;
        return trigger;
    }
    if (preset == "ZoneChangeStageWr") {
        auto trigger = asn::Trigger{};
        auto tr = asn::ZoneChangeTrigger();
        tr.target.push_back(thisTarget);
        tr.from = asn::Zone::Stage;
        tr.to = asn::Zone::WaitingRoom;
        trigger.trigger = tr;
        trigger.type = asn::TriggerType::OnZoneChange;
        return trigger;
    }
    if (preset == "ZoneChangeCxPlaced") {
        auto trigger = asn::Trigger{};
        auto tr = asn::ZoneChangeTrigger();
        asn::Card card;
        asn::CardSpecifier spec;
        spec.type = asn::CardSpecifierType::CardType;
        spec.specifier = asn::CardType::Climax;
        card.cardSpecifiers.push_back((spec));
        asn::Target target{
            .type = asn::TargetType::SpecificCards,
            .targetSpecification = asn::TargetSpecificCards{
                .mode = asn::TargetMode::Any,
                .number = asn::Number{.mod=asn::NumModifier::ExactMatch,
                                      .value=1},
                .cards = card
            }
        };
        tr.target.push_back(target);
        tr.from = asn::Zone::NotSpecified;
        tr.to = asn::Zone::Climax;
        trigger.trigger = tr;
        trigger.type = asn::TriggerType::OnZoneChange;
        return trigger;
    }
    if (preset == "OnAttack") {
        auto trigger = asn::Trigger{};
        trigger.type = asn::TriggerType::OnAttack;
        trigger.trigger = asn::OnAttackTrigger{.target = thisTarget};
        return trigger;
    }
    if (preset == "OnReverseThis") {
        auto trigger = asn::Trigger{};
        trigger.type = asn::TriggerType::OnStateChange;
        trigger.trigger = asn::StateChangeTrigger{
            .state=asn::State::Reversed,
            .target=thisTarget
        };
        return trigger;
    }
    if (preset == "OnReverseOpp") {
        auto trigger = asn::Trigger{};
        trigger.type = asn::TriggerType::OnStateChange;
        auto target = asn::Target{.type=asn::TargetType::BattleOpponent,
                .targetSpecification=emptyTargetSpec};
        auto tr = asn::StateChangeTrigger{
            .state=asn::State::Reversed,
            .target=target
        };
        trigger.trigger = tr;
        return trigger;
    }
    if (preset == "YourCxPhase") {
        auto trigger = asn::Trigger{};
        trigger.type = asn::TriggerType::OnPhaseEvent;
        trigger.trigger = asn::PhaseTrigger{
            .phase=asn::Phase::ClimaxPhase,
            .state=asn::PhaseState::Start,
            .player=asn::Player::Player
        };
        return trigger;
    }
    if (preset == "YourEncore") {
        auto trigger = asn::Trigger{};
        trigger.type = asn::TriggerType::OnPhaseEvent;
        trigger.trigger = asn::PhaseTrigger{
            .phase=asn::Phase::EncoreStep,
            .state=asn::PhaseState::Start,
            .player=asn::Player::Player
        };
        return trigger;
    }
    if (preset == "OppAttackPhase") {
        auto trigger = asn::Trigger{};
        trigger.type = asn::TriggerType::OnPhaseEvent;
        trigger.trigger = asn::PhaseTrigger{
            .phase=asn::Phase::AttackPhase,
            .state=asn::PhaseState::Start,
            .player=asn::Player::Opponent
        };
        return trigger;
    }
    if (preset == "EndOfCardsAttack") {
        auto trigger = asn::Trigger{};
        trigger.type = asn::TriggerType::OnEndOfThisCardsAttack;
        return trigger;
    }
    return std::nullopt;
}

void TriggerComponent::setTriggerInQml() {
    gen::TriggerHelper helper(this);
    helper.setTriggerInQml(type_, trigger_);
}

