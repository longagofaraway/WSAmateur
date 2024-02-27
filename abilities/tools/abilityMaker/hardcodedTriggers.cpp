#include "hardcodedAbilities.h"


asn::Trigger fromHandToStagePreset() {
    auto tr = asn::ZoneChangeTrigger();
    tr.from = asn::Zone::Hand;
    tr.to = asn::Zone::Stage;
    auto targ = asn::Target();
    targ.type = asn::TargetType::ThisCard;
    tr.target.push_back(targ);

    return asn::Trigger{
      .type = asn::TriggerType::OnZoneChange,
      .trigger = tr
    };
}

asn::Trigger thisCardAttacks() {
    auto tr = asn::OnAttackTrigger();
    auto targ = asn::Target();
    targ.type = asn::TargetType::ThisCard;
    tr.target = targ;

    return asn::Trigger{
      .type = asn::TriggerType::OnAttack,
      .trigger = tr
    };
}

asn::Trigger thisCardBecomesReversed() {
    auto tr = asn::StateChangeTrigger();
    auto targ = asn::Target();
    targ.type = asn::TargetType::ThisCard;
    tr.target = targ;
    tr.state = asn::State::Reversed;

    return asn::Trigger{
      .type = asn::TriggerType::OnStateChange,
      .trigger = tr
    };
}

asn::Trigger climaxIsPlaced() {
    asn::Card card;
    card.cardSpecifiers.push_back(asn::CardSpecifier{
        .type = asn::CardSpecifierType::CardType,
        .specifier = asn::CardType::Climax
    });
    auto tr = asn::ZoneChangeTrigger();
    auto targ = asn::Target();
    targ.type = asn::TargetType::SpecificCards;
    targ.targetSpecification = asn::TargetSpecificCards{
        .mode = asn::TargetMode::Any,
        .number = asn::Number{
            .mod = asn::NumModifier::ExactMatch,
            .value = 1
        },
        .cards = card
    };
    tr.target.push_back(targ);
    tr.from = asn::Zone::NotSpecified;
    tr.to = asn::Zone::Climax;

    return asn::Trigger{
      .type = asn::TriggerType::OnZoneChange,
      .trigger = tr
    };
}

asn::Trigger startOfOppAttackPhase() {
    auto tr = asn::PhaseTrigger{
        .phase = asn::Phase::AttackPhase,
        .state = asn::PhaseState::Start,
        .player = asn::Player::Opponent
    };

    return asn::Trigger{
      .type = asn::TriggerType::OnPhaseEvent,
      .trigger = tr
    };
}
