#include "card.h"

#include "ability_maker_gen.h"

namespace {

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

CardComponent::CardComponent(QString moduleName, QQuickItem *parent, QString id, QString displayName)
    : BaseComponent(moduleName, parent, id) {
    mediator = std::make_shared<gen::ComponentMediator>(this);
    connect(qmlObject_, SIGNAL(createCardSpecifier(QString,QString)), this, SLOT(onCreateCardSpecifier(QString,QString)));
    qmlObject_->setProperty("displayName", displayName);
}

void CardComponent::refitComponents() {
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

void CardComponent::addComponent(QQuickItem* object, QString componentId, const asn::CardSpecifier& specifier) {
    cardSpecifiers_[componentId] = CardSpecifier{.object=object,.specifier=specifier};
    refitComponents();
}

void CardComponent::onCreateCardSpecifier(QString cardSpecifierType, QString value) {
    auto cardSpecifier = buildCardSpecifier(cardSpecifierType, value);
    createCardSpecifier(cardSpecifier);
    createCardSpecifierAdditionalActions();
}

void CardComponent::createCardSpecifier(const asn::CardSpecifier& specifier) {
    auto componentId = generateCardSpecifierId();

    auto *object = componentManager_.createComponent("CardSpecifier", "CardSpecifier", componentId, qmlObject_, this, mediator.get());
    addComponent(object, componentId, specifier);
    emit setCardSpecifier(specifier, componentId);
}

void CardComponent::deleteCardSpecifier(QString componentId) {
    componentManager_.deleteComponent(componentId);
    cardSpecifiers_.remove(componentId);
    refitComponents();
    notifyOfChanges();
}

void CardComponent::cardSpecifierChanged(const asn::CardSpecifier& specifier, QString componentId) {
    cardSpecifiers_[componentId].specifier = specifier;
    notifyOfChanges();
}

void CardComponent::setCard(const asn::Card& card) {
    for (const auto& cardSpec: card.cardSpecifiers) {
        createCardSpecifier(cardSpec);
    }
}

void CardComponent::notifyOfChanges() {
    asn::Card card;
    for (auto& value: cardSpecifiers_) {
        card.cardSpecifiers.push_back(value.specifier);
    }
    emit cardReady(card, componentId_);
}
