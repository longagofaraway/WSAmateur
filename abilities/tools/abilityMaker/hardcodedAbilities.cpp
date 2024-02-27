#include "hardcodedAbilities.h"

namespace {
asn::Condition noCondition() {
    return asn::Condition{
        .type = asn::ConditionType::NoCondition
    };
}
asn::Card cardWithCardType(asn::CardType type) {
    std::vector<asn::CardSpecifier> specs;
    specs.push_back(asn::CardSpecifier{
        .type = asn::CardSpecifierType::CardType,
        .specifier = type
    });
    return asn::Card{
        .cardSpecifiers = specs
    };
}
asn::Target simpleTarget(asn::TargetType type) {
    return asn::Target{
        .type = type
    };
}
asn::Target targetCard() {
    return asn::Target{
        .type = asn::TargetType::SpecificCards,
        .targetSpecification = asn::TargetSpecificCards{
            .mode = asn::TargetMode::Any,
            .number = asn::Number{
                .mod = asn::NumModifier::ExactMatch,
                .value = 1
            }
        }
    };
}
asn::Target targetCardType(asn::CardType type) {
    return asn::Target{
        .type = asn::TargetType::SpecificCards,
        .targetSpecification = asn::TargetSpecificCards{
            .mode = asn::TargetMode::Any,
            .number = asn::Number{
                .mod = asn::NumModifier::ExactMatch,
                .value = 1
            },
            .cards = cardWithCardType(type)
        }
    };
}
asn::Effect searchChar() {
    std::vector<asn::Card> card;
    card.push_back(cardWithCardType(asn::CardType::Char));

    std::vector<asn::SearchTarget> targets;
    targets.push_back(asn::SearchTarget{
        .number=asn::Number{
            .mod = asn::NumModifier::UpTo,
            .value = 1
        },
        .cards = card
    });

    asn::Effect effect;
    effect.type = asn::EffectType::SearchCard;
    effect.cond = noCondition();
    effect.effect = asn::SearchCard{
        .targets = targets,
        .place = asn::Place{
            .pos = asn::Position::NotSpecified,
            .zone = asn::Zone::Deck,
            .owner = asn::Player::Player
        }
    };
    return effect;
}
asn::Effect reveal() {
    asn::Effect effect;
    effect.type = asn::EffectType::RevealCard;
    effect.cond = noCondition();
    effect.effect = asn::RevealCard{
        .type = asn::RevealType::ChosenCards,
        .number = asn::Number{}
    };
    return effect;
}
asn::Effect putIntoHand() {
    asn::Effect effect;
    effect.type = asn::EffectType::MoveCard;
    effect.cond = noCondition();
    effect.effect = asn::MoveCard{
        .executor = asn::Player::Player,
        .target = asn::Target{
            .type = asn::TargetType::ChosenCards
        },
        .from = asn::Place{},
        .to = std::vector<asn::Place>{
            asn::Place{
                .pos=asn::Position::NotSpecified,
                .zone=asn::Zone::Hand,
                .owner=asn::Player::Player
            }
        }
    };
    return effect;
}
asn::Effect shuffle() {
    asn::Effect effect;
    effect.type = asn::EffectType::Shuffle;
    effect.cond = noCondition();
    effect.effect = asn::Shuffle{
        .zone = asn::Zone::Deck,
        .owner = asn::Player::Player
    };
    return effect;
}
asn::Effect draw() {
    asn::Effect effect;
    effect.type = asn::EffectType::DrawCard;
    effect.cond = noCondition();
    effect.effect = asn::DrawCard{
        .executor = asn::Player::Player,
        .value = asn::Number{
            .mod = asn::NumModifier::ExactMatch,
            .value = 1
        }
    };
    return effect;
}
asn::TargetAndPlace charInWr() {
    asn::TargetAndPlace tp;
    tp.target = targetCardType(asn::CardType::Char);
    tp.placeType = asn::PlaceType::SpecificPlace;
    tp.place = asn::Place{
        .pos = asn::Position::NotSpecified,
        .zone = asn::Zone::WaitingRoom,
        .owner = asn::Player::Player
    };
    return tp;
}
asn::Effect chosenToHand() {
    std::vector<asn::Place> places;
    places.push_back(asn::Place{
                         .pos = asn::Position::NotSpecified,
                         .zone = asn::Zone::Hand,
                         .owner = asn::Player::Player
                     });
    asn::Effect effect;
    effect.type = asn::EffectType::MoveCard;
    effect.cond = noCondition();
    effect.effect = asn::MoveCard{
        .executor = asn::Player::Player,
        .target = simpleTarget(asn::TargetType::ChosenCards),
        .from = asn::Place{
            .pos = asn::Position::NotSpecified,
            .zone = asn::Zone::Stage,
            .owner = asn::Player::Player
        },
        .to = places
    };
    return effect;
}
std::vector<asn::Effect> chooseSalvageChar() {
    std::vector<asn::TargetAndPlace> tps;
    tps.push_back(charInWr());
    std::vector<asn::Effect> effects;
    asn::Effect effect;
    effect.type = asn::EffectType::ChooseCard;
    effect.cond = noCondition();
    effect.effect = asn::ChooseCard{
        .executor = asn::Player::Player,
        .targets = tps
    };
    effects.push_back(effect);
    effects.push_back(chosenToHand());
    return effects;
};
asn::Effect payCostSalvageChar() {
    asn::Effect effect;
    effect.type = asn::EffectType::PayCost;
    effect.cond = noCondition();
    effect.effect = asn::PayCost{
            .ifYouDo = chooseSalvageChar()
    };
    return effect;
}
std::vector<asn::Effect> effectBrainstormDeck() {
    std::vector<asn::Effect> effects;
    effects.push_back(searchChar());
    effects.push_back(reveal());
    effects.push_back(putIntoHand());
    effects.push_back(shuffle());
    return effects;
}
std::vector<asn::Effect> effectBrainstormDraw() {
    std::vector<asn::Effect> effects;
    effects.push_back(draw());
    return effects;
}
asn::Effect restThis() {
    asn::Effect effect;
    effect.type = asn::EffectType::ChangeState;
    effect.cond = noCondition();
    effect.effect = asn::ChangeState{
            .target = asn::Target{
                .type = asn::TargetType::ThisCard
            },
            .state = asn::State::Rested
    };
    return effect;
}
asn::Effect discardCard() {
    asn::Effect effect;
    effect.type = asn::EffectType::MoveCard;
    effect.cond = noCondition();
    effect.effect = asn::MoveCard{
        .executor = asn::Player::Player,
        .target = targetCard(),
        .from = asn::Place{
            .pos = asn::Position::NotSpecified,
            .zone = asn::Zone::Hand,
            .owner = asn::Player::Player
        },
        .to = std::vector<asn::Place>{
            asn::Place{
                .pos = asn::Position::NotSpecified,
                .zone = asn::Zone::WaitingRoom,
                .owner = asn::Player::Player
            }
        }
    };
    return effect;
}
std::vector<asn::Effect> returnAsRest() {
    std::vector<asn::Effect> ifYouDo;
    ifYouDo.push_back(asn::Effect{
          .type = asn::EffectType::MoveCard,
          .cond = noCondition(),
          .effect = asn::MoveCard{
              .executor = asn::Player::Player,
              .target = asn::Target{
                  .type = asn::TargetType::ThisCard
              },
              .to = std::vector<asn::Place>{
                  asn::Place{
                      .pos = asn::Position::SlotThisWasInRested,
                      .zone = asn::Zone::Stage,
                      .owner = asn::Player::Player
                  }
              }
          }
      });

    std::vector<asn::Effect> effects;
    asn::Effect pay;
    pay.type = asn::EffectType::PayCost;
    pay.cond = noCondition();
    pay.effect = asn::PayCost{
        .ifYouDo = ifYouDo
    };
    effects.push_back(pay);
    return effects;
}
asn::Ability brainstorm(int type) {
    asn::Ability a;
    a.type = asn::AbilityType::Act;

    asn::ActAbility act{};
    act.keywords.push_back(asn::Keyword::Brainstorm);
    act.cost.items.push_back(asn::CostItem{
      .type = asn::CostType::Stock,
      .costItem = asn::StockCost{.value=1}
    });
    act.cost.items.push_back(asn::CostItem{
      .type = asn::CostType::Effects,
      .costItem = restThis()
    });
    std::vector<asn::Effect> effects;
    switch (type) {
    case 0:
        effects = effectBrainstormDeck();
        break;
    case 1:
        effects = effectBrainstormDraw();
        break;
    }

    act.effects.push_back(asn::Effect{
                              .type = asn::EffectType::FlipOver,
                              .cond = noCondition(),
                              .effect = asn::FlipOver{
                                  .number = asn::Number{
                                      .mod = asn::NumModifier::ExactMatch,
                                      .value = 4
                                  },
                                  .forEach = cardWithCardType(asn::CardType::Climax),
                                  .effect = effects
                              }
    });
    a.ability = act;

    return a;
}
}

asn::Ability brainstormDeck() {
    return brainstorm(0);
}

asn::Ability brainstormDraw() {
    return brainstorm(1);
}

asn::Ability encore() {
    asn::AutoAbility aut{};
    aut.keywords.push_back(asn::Keyword::Encore);

    asn::Cost cost;
    cost.items.push_back(asn::CostItem{
      .type = asn::CostType::Effects,
      .costItem = discardCard()
    });
    aut.cost = cost;

    std::vector<asn::Target> targets;
    targets.push_back(asn::Target{
        .type = asn::TargetType::ThisCard
    });

    std::vector<asn::Trigger> triggers;
    triggers.push_back(asn::Trigger{
       .type = asn::TriggerType::OnZoneChange,
       .trigger = asn::ZoneChangeTrigger{
           .target = targets,
           .from = asn::Zone::Stage,
           .to = asn::Zone::WaitingRoom
       }
    });

    aut.triggers = triggers;
    aut.effects = returnAsRest();

    asn::Ability a;
    a.type = asn::AbilityType::Auto;
    a.ability = aut;
    return a;
}

asn::Ability bond() {
    asn::AutoAbility aut{};
    aut.keywords.push_back(asn::Keyword::Bond);

    asn::Cost cost;
    cost.items.push_back(asn::CostItem{
      .type = asn::CostType::Effects,
      .costItem = discardCard()
    });
    aut.cost = cost;

    std::vector<asn::Trigger> triggers;
    triggers.push_back(asn::Trigger{
       .type = asn::TriggerType::OnPlay,
       .trigger = asn::OnPlayTrigger{
           .target = asn::Target{
               .type = asn::TargetType::ThisCard,
           }
       }
    });
    aut.triggers = triggers;
    std::vector<asn::Effect> effects;
    effects.push_back(payCostSalvageChar());
    aut.effects = effects;

    asn::Ability a;
    a.type = asn::AbilityType::Auto;
    a.ability = aut;
    return a;
}
