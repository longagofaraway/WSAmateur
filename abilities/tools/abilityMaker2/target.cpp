#include "target.h"

#include <variant>

#include "language_parser.h"
#include "languageSpecification.h"

TargetComponent::TargetComponent(QQuickItem *parent, QString id, QString displayName)
    : CardComponent("Target", parent, id, displayName) {
    connect(qmlObject_, SIGNAL(numModifierChanged(QString)), this, SLOT(onNumModifierChanged(QString)));
    connect(qmlObject_, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
    connect(qmlObject_, SIGNAL(setTargetMode(QString)), this, SLOT(setTargetMode(QString)));
    connect(qmlObject_, SIGNAL(setTargetType(QString)), this, SLOT(setTargetType(QString)));
}


void TargetComponent::createCardSpecifierAdditionalActions() {
    if (type_ != asn::TargetType::SpecificCards && type_ != asn::TargetType::BattleOpponent) {
        type_ = asn::TargetType::SpecificCards;
        number_ = asn::Number{.mod=asn::NumModifier::ExactMatch,.value=1};
        setQmlNumber(number_.value());
    }
    setSecondLine();
}

void TargetComponent::setSecondLine() {
    auto secondLine = toString(type_);
    if (type_ == asn::TargetType::SpecificCards) {
        if (mode_ != asn::TargetMode::Any) {
            secondLine = toString(mode_);
        } else {
            secondLine = toString(type_);
        }
    }
    QMetaObject::invokeMethod(qmlObject_, "setSecondLine", Q_ARG(QVariant, QString::fromStdString(secondLine)));
}

void TargetComponent::setQmlNumber(const asn::Number& number) {
    QMetaObject::invokeMethod(qmlObject_, "showNumber");
    QMetaObject::invokeMethod(qmlObject_, "setNumMod", Q_ARG(QVariant, QString::fromStdString(toString(number_.value().mod))));
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::number(number_.value().value)));
}

void TargetComponent::setTarget(asn::Target target) {
    type_ = target.type;
    if (target.targetSpecification.has_value()) {
        const auto& spec = target.targetSpecification.value();
        number_ = spec.number;
        mode_ = spec.mode;
        setQmlNumber(number_.value());

        for (const auto& cardSpec: spec.cards.cardSpecifiers) {
            createCardSpecifier(cardSpec);
        }
    } else {
        QMetaObject::invokeMethod(qmlObject_, "hideNumber");
    }
    setSecondLine();
    notifyOfChanges();
}

void TargetComponent::onNumModifierChanged(QString numModifier) {
    number_.value().mod = parse(numModifier.toStdString(), formats::To<asn::NumModifier>{});
    notifyOfChanges();
}

void TargetComponent::onNumValueChanged(QString value) {
    number_.value().value = value.toInt();
    notifyOfChanges();
}

void TargetComponent::setTargetMode(QString targetMode) {
    mode_ = parse(targetMode.toStdString(), formats::To<asn::TargetMode>{});
    type_ = asn::TargetType::SpecificCards;
    number_ = asn::Number{.mod=asn::NumModifier::ExactMatch,.value=1};
    QMetaObject::invokeMethod(qmlObject_, "showNumber");
    notifyOfChanges();
    setSecondLine();
}

void TargetComponent::setTargetType(QString targetType) {
    auto oldType = type_;
    type_ = parse(targetType.toStdString(), formats::To<asn::TargetType>{});
    if (type_ != asn::TargetType::SpecificCards && type_ != asn::TargetType::BattleOpponent) {
        for (auto& key: cardSpecifiers_.keys()) {
            componentManager_.deleteComponent(key);
        }
        cardSpecifiers_.clear();
        refitComponents();
        QMetaObject::invokeMethod(qmlObject_, "hideNumber");
        number_ = std::nullopt;
    } else if (oldType != asn::TargetType::SpecificCards && oldType != asn::TargetType::BattleOpponent) {
        number_ = asn::Number{.mod=asn::NumModifier::ExactMatch,.value=1};
        setQmlNumber(number_.value());
    }
    notifyOfChanges();
    setSecondLine();
}

void TargetComponent::notifyOfChanges() {
    asn::Target target;
    target.type = type_;
    if (number_.has_value()) {
        asn::TargetSpecificCards spec;
        spec.mode = mode_;
        spec.number = number_.value();
        for (auto& value: cardSpecifiers_) {
            spec.cards.cardSpecifiers.push_back(value.specifier);
        }
        target.targetSpecification = spec;
    }
    emit targetReady(target, componentId_);
}
