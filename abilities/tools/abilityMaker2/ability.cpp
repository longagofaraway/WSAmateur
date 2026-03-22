#include "ability.h"

#include <variant>

#include <QList>
#include <QVector>

#include "effectsTree.h"
#include "trigger.h"
#include "triggerInit.h"
#include "language_parser.h"

// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

template<class... Ts>
overloads(Ts...) -> overloads<Ts...>;

using value_t = std::variant<int, long, double, std::string>;

void AbilityComponent::componentComplete() {
    QQuickItem::componentComplete();
}

void AbilityComponent::createTrigger(QString triggerId, QQuickItem *parent) {
    auto trigger = getTriggerFromPreset(triggerId);
    if (trigger.has_value()) {
        QMetaObject::invokeMethod(this, "setTriggerText", Q_ARG(QString, QString::fromStdString(toString(trigger.value().type))));
        triggers_.clear();
        triggers_.push_back(trigger.value());
        openTrigger(parent);
    } else {
        currentComponent_ = std::make_shared<TriggerComponent>(parent);
    }
}

void AbilityComponent::openTrigger(QQuickItem *parent) {
    auto triggerComponent = std::make_shared<TriggerComponent>(parent, triggers_);
    connect(&*triggerComponent, &TriggerComponent::componentChanged, this, &AbilityComponent::triggersChanged);
    currentComponent_ = triggerComponent;
}

void AbilityComponent::updateKeywords(QVariant keywordList) {
    keywords_.clear();
    auto list = keywordList.toList();
    for (int i = 0; i < list.size(); ++i)
        keywords_.push_back(parse(list[i].toString().toStdString(), formats::To<asn::Keyword>()));
    emit componentChanged(constructAbility(), id_);
}

void AbilityComponent::setCurrentComponent(std::shared_ptr<BaseComponent> component) {
    currentComponent_ = component;
}

void AbilityComponent::subscribeToEffectsChange(EffectsTree* effectsTree) {
    connect(effectsTree, &EffectsTree::componentChanged, this, &AbilityComponent::effectsChanged);
}

void AbilityComponent::triggersChanged(const std::vector<asn::Trigger>& triggers) {
    auto triggerText = QString::fromStdString(toString(triggers.at(0).type));
    if (triggers.size() > 1) triggerText =+ "...";
    QMetaObject::invokeMethod(this, "setTriggerText", Q_ARG(QString, triggerText));
    triggers_ = triggers;

    emit componentChanged(constructAbility(), id_);
}

void AbilityComponent::effectsChanged(const std::vector<asn::Effect>& effects) {
    effects_ = effects;

    emit componentChanged(constructAbility(), id_);
}


asn::Ability AbilityComponent::constructAbility() {
    asn::Ability ability;
    ability.type = type_;
    switch (type_) {
    case asn::AbilityType::Auto:
        ability.ability = constructSpecificAbility<asn::AutoAbility>();
        break;
    case asn::AbilityType::Cont:
        ability.ability = constructSpecificAbility<asn::ContAbility>();
        break;
    case asn::AbilityType::Act:
        ability.ability = constructSpecificAbility<asn::ActAbility>();
        break;
    case asn::AbilityType::Event:
        ability.ability = constructSpecificAbility<asn::EventAbility>();
        break;
    }
    return ability;
}

void AbilityComponent::setAbility(const asn::Ability& ability, QString id) {
    id_ = id;
    type_ = ability.type;

    std::visit( overloads
    {
        [this](const asn::AutoAbility& ability) {
            triggers_ = ability.triggers;
            effects_ = ability.effects;
            cost_ = ability.cost;
            keywords_ = ability.keywords;
            activationTimes_ = ability.activationTimes;
        },
        [this](const asn::ContAbility& ability) {
            effects_ = ability.effects;
            keywords_ = ability.keywords;
        },
        [this](const asn::ActAbility& ability) {
            effects_ = ability.effects;
            cost_ = ability.cost;
        },
        [this](const asn::EventAbility& ability) {
            effects_ = ability.effects;
            keywords_ = ability.keywords;
        },
        [](auto arg) {
            qWarning() << "unhandled ability type";
            throw std::runtime_error("unhandled ability type");
        }
    }, ability.ability);

    QMetaObject::invokeMethod(this, "setAbilityType", Q_ARG(QVariant, QString::fromStdString(toString(ability.type))));

    std::vector<QString> vec;
    std::transform(keywords_.begin(), keywords_.end(), std::back_inserter(vec), [](auto value){ return QString::fromStdString(toString(value)); });
    QList<QString> myList;
    std::copy(vec.begin(), vec.end(), std::back_inserter(myList));
    QMetaObject::invokeMethod(this, "setKeywords", Q_ARG(QVariant, QVariant::fromValue(myList)));
}
