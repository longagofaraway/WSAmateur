#include "quickSetters.h"


namespace {
template<typename T>
void addEffectCost(asn::EffectType type, T &&effect, asn::Cost &cost) {
    asn::Condition condition;
    condition.type = asn::ConditionType::NoCondition;

    asn::Effect metaEffect;
    metaEffect.type = type;
    metaEffect.effect = effect;
    metaEffect.cond = condition;

    asn::CostItem item;
    item.type = asn::CostType::Effects;
    item.costItem = metaEffect;

    cost.items.push_back(item);
}
}

void addCostItem(asn::Cost &cost, QString costName) {
    if (costName == "Add stock cost") {
        for (auto &item: cost.items) {
            if (item.type == asn::CostType::Stock) {
                auto &stock = std::get<asn::StockCost>(item.costItem);
                stock.value++;
                return;
            }
        }
        asn::StockCost stock;
        stock.value = 1;

        asn::CostItem item;
        item.costItem = stock;
        item.type = asn::CostType::Stock;
        cost.items.push_back(item);
        return;
    }
    if (costName == "Add discard a card") {
        asn::MoveCard moveCard;
        moveCard.executor = asn::Player::Player;
        moveCard.from = asn::Place{
                .pos=asn::Position::NotSpecified,
                .zone=asn::Zone::Hand,
                .owner=asn::Player::Player
        };
        asn::Place to{
            .pos=asn::Position::NotSpecified,
            .zone=asn::Zone::WaitingRoom,
            .owner=asn::Player::Player
        };
        moveCard.to.push_back(to);
        moveCard.order = asn::Order::NotSpecified;

        asn::Target target;
        target.type = asn::TargetType::SpecificCards;
        asn::TargetSpecificCards spec;
        spec.mode = asn::TargetMode::Any;
        spec.number = asn::Number{
                .mod=asn::NumModifier::ExactMatch,
                .value=1
        };
        target.targetSpecification = spec;
        moveCard.target = target;

        addEffectCost(asn::EffectType::MoveCard, moveCard, cost);
        return;
    }
    if (costName == "Add rest this") {
        asn::ChangeState changeState;
        changeState.state = asn::State::Rested;
        changeState.target.type = asn::TargetType::ThisCard;
        addEffectCost(asn::EffectType::ChangeState, changeState, cost);
        return;
    }
}
