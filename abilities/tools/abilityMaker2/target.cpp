#include "target.h"

#include <variant>

#include "language_parser.h"
#include "languageSpecification.h"

namespace {

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

QString generateCardSpecifierId() {
    static int i = 0;
    return QString("cardSpecifier_") + QString::number(i++);
}

asn::CardSpecifier buildCardSpecifier(QString cardSpecifierType, QString value) {
    asn::CardSpecifier spec;
    spec.type = parse(cardSpecifierType.toStdString(), formats::To<asn::CardSpecifierType>{});

    asn::Number defaultLevel = asn::Number{.mod=asn::NumModifier::UpTo,.value=3};
    asn::Number defaultCost = asn::Number{.mod=asn::NumModifier::ExactMatch,.value=0};

    switch (spec.type) {
    case asn::CardSpecifierType::CardType:
        spec.specifier = parse(value.toStdString(), formats::To<asn::CardType>{});
        break;
    case asn::CardSpecifierType::Owner:
        spec.specifier = parse(value.toStdString(), formats::To<asn::Player>{});
        break;
    case asn::CardSpecifierType::Trait:
        spec.specifier = asn::Trait{};
        break;
    case asn::CardSpecifierType::ExactName:
        spec.specifier = asn::ExactName{};
        break;
    case asn::CardSpecifierType::NameContains:
        spec.specifier = asn::NameContains{};
        break;
    case asn::CardSpecifierType::Level:
        spec.specifier = asn::Level{defaultLevel};
        break;
    case asn::CardSpecifierType::LevelHigherThanOpp:
        spec.specifier = std::monostate{};
        break;
    case asn::CardSpecifierType::Color:
        spec.specifier = parse(value.toStdString(), formats::To<asn::Color>{});
        break;
    case asn::CardSpecifierType::Cost:
        spec.specifier = asn::CostSpecifier{defaultCost};
        break;
    case asn::CardSpecifierType::TriggerIcon:
        spec.specifier = parse(value.toStdString(), formats::To<asn::TriggerIcon>{});
        break;
    case asn::CardSpecifierType::HasMarker:
        spec.specifier = std::monostate{};
        break;
    case asn::CardSpecifierType::Power:
        spec.specifier = asn::Power{defaultCost};
        break;
    case asn::CardSpecifierType::StandbyTarget:
        spec.specifier = std::monostate{};
        break;
    case asn::CardSpecifierType::LevelWithMultiplier:
        spec.specifier = asn::LevelWithMultiplier{.value=defaultCost};
        break;
    case asn::CardSpecifierType::State:
        spec.specifier = parse(value.toStdString(), formats::To<asn::State>{});
        break;
    default:
        throw std::runtime_error("unhandled CardSpecifierType");
    }

    return spec;
}
} // namespace

TargetComponent::TargetComponent(QQuickItem *parent, QString id, QString displayName)
    : BaseComponent("Target", parent, id) {
    connect(qmlObject_, SIGNAL(createCardSpecifier(QString,QString)), this, SLOT(onCreateCardSpecifier(QString,QString)));
    connect(qmlObject_, SIGNAL(numModifierChanged(QString)), this, SLOT(onNumModifierChanged(QString)));
    connect(qmlObject_, SIGNAL(numValueChanged(QString)), this, SLOT(onNumValueChanged(QString)));
    connect(qmlObject_, SIGNAL(setTargetMode(QString)), this, SLOT(setTargetMode(QString)));
    connect(qmlObject_, SIGNAL(setTargetType(QString)), this, SLOT(setTargetType(QString)));
    qmlObject_->setProperty("displayName", displayName);
    number_.mod = asn::NumModifier::AtLeast;
    number_.value = 1;
}

void TargetComponent::refitComponents() {
    if (cardSpecifiers_.empty()) return;

    int i = 0;
    auto middle = qmlObject_->width() / 3;
    QQuickItem *bottomObject = qmlObject_;
    qreal margin = 50;
    int maxSize = std::min(cardSpecifiers_.size(), 2);
    qreal fullLength = maxSize * cardSpecifiers_.first().object->width() + (maxSize - 1) * margin;
    auto getX = [&](QQuickItem* obj, int index) {
        return middle - fullLength / 2 + (index % 2) * (obj->width() + margin);
    };
    for (auto it = cardSpecifiers_.begin(); it != cardSpecifiers_.end(); ++it) {
        it.value().object->setX(getX(it.value().object, i));
        qvariant_cast<QObject*>(it.value().object->property("anchors"))->setProperty("top", bottomObject->property("bottom"));
        qvariant_cast<QObject*>(it.value().object->property("anchors"))->setProperty("topMargin", QVariant(10));
        i++;
        if (i % 2 == 0) bottomObject = it.value().object;
    }
}

void TargetComponent::addComponent(QQuickItem* object, QString componentId, const asn::CardSpecifier& specifier) {
    cardSpecifiers_[componentId] = CardSpecifier{.object=object,.specifier=specifier};
    refitComponents();
}

void TargetComponent::onCreateCardSpecifier(QString cardSpecifierType, QString value) {
    if (type_ != asn::TargetType::SpecificCards && type_ != asn::TargetType::BattleOpponent) {
        type_ = asn::TargetType::SpecificCards;
        QMetaObject::invokeMethod(qmlObject_, "showNumber");
    }
    auto cardSpecifier = buildCardSpecifier(cardSpecifierType, value);
    createCardSpecifier(cardSpecifier);
    setSecondLine();
}

void TargetComponent::createCardSpecifier(const asn::CardSpecifier& specifier) {
    auto componentId = generateCardSpecifierId();

    auto *object = componentManager_.createComponent("CardSpecifier", "CardSpecifier", componentId, qmlObject_, this);
    addComponent(object, componentId, specifier);
    emit setCardSpecifier(specifier, componentId);
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

void TargetComponent::setTarget(const asn::Target& target) {
    type_ = target.type;
    if (type_ == asn::TargetType::SpecificCards) {
        QMetaObject::invokeMethod(qmlObject_, "showNumber");
    } else {
        QMetaObject::invokeMethod(qmlObject_, "hideNumber");
    }
    if (target.targetSpecification.has_value()) {
        const auto& spec = target.targetSpecification.value();
        number_ = spec.number;
        mode_ = spec.mode;
        QMetaObject::invokeMethod(qmlObject_, "setNumMod", Q_ARG(QVariant, QString::fromStdString(toString(number_.mod))));
        QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::number(number_.value)));

        for (const auto& cardSpec: spec.cards.cardSpecifiers) {
            createCardSpecifier(cardSpec);
        }
    }
    setSecondLine();
}

void TargetComponent::deleteCardSpecifier(QString componentId) {
    componentManager_.deleteComponent(componentId);
    cardSpecifiers_.remove(componentId);
    refitComponents();
    buildTarget();
}

void TargetComponent::onNumModifierChanged(QString numModifier) {
    number_.mod = parse(numModifier.toStdString(), formats::To<asn::NumModifier>{});
    buildTarget();
}

void TargetComponent::onNumValueChanged(QString value) {
    number_.value = value.toInt();
    buildTarget();
}

void TargetComponent::setTargetMode(QString targetMode) {
    mode_ = parse(targetMode.toStdString(), formats::To<asn::TargetMode>{});
    type_ = asn::TargetType::SpecificCards;
    QMetaObject::invokeMethod(qmlObject_, "showNumber");
    buildTarget();
    setSecondLine();
}

void TargetComponent::setTargetType(QString targetType) {
    type_ = parse(targetType.toStdString(), formats::To<asn::TargetType>{});
    if (type_ != asn::TargetType::SpecificCards && type_ != asn::TargetType::BattleOpponent) {
        for (auto& key: cardSpecifiers_.keys()) {
            componentManager_.deleteComponent(key);
        }
        cardSpecifiers_.clear();
        refitComponents();
        QMetaObject::invokeMethod(qmlObject_, "hideNumber");
    }
    buildTarget();
    setSecondLine();
}

void TargetComponent::cardSpecifierChanged(const asn::CardSpecifier& specifier, QString componentId) {
    cardSpecifiers_[componentId].specifier = specifier;
    buildTarget();
}

void TargetComponent::buildTarget() {
    asn::Target target;
    target.type = type_;
    if (type_ == asn::TargetType::SpecificCards || type_ == asn::TargetType::BattleOpponent) {
        asn::TargetSpecificCards spec;
        spec.mode = mode_;
        spec.number = number_;
        for (auto& value: cardSpecifiers_) {
            spec.cards.cardSpecifiers.push_back(value.specifier);
        }
    }
    emit targetReady(target, componentId_);
}
