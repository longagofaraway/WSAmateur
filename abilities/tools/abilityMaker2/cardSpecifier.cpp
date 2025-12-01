#include "cardSpecifier.h"

#include <QQmlContext>

#include "componentHelper.h"
#include "language_parser.h"
#include "languageSpecification.h"

namespace {

QQuickItem* createObject(const QString &moduleName, QQuickItem *parent, QString componentName) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + moduleName + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem* object = qobject_cast<QQuickItem*>(obj);
    object->setParentItem(parent);
    object->setParent(parent);
    object->setProperty("componentName", componentName);
    qvariant_cast<QObject*>(object->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    return object;
}

}

CardSpecifierComponent::CardSpecifierComponent(QQuickItem *parent, QString componentId)
    : BaseComponent("CardSpecifier", parent, componentId) {
    componentId_ = componentId;
    connect(qmlObject_, SIGNAL(removeCardSpecifier()), this, SLOT(deleteItself()));
}

CardSpecifierComponent::~CardSpecifierComponent() {
    qmlSpecifier_->deleteLater();
}

void CardSpecifierComponent::deleteItself() {
    emit deleteComponent(componentId_);
}

void CardSpecifierComponent::setCardSpecifier(const asn::CardSpecifier& cardSpecifier) {
    specifier_ = cardSpecifier;

    auto cardSpecifierTypeName = QString::fromStdString(toString(cardSpecifier.type));
    qDebug() << "creating specifier " << cardSpecifierTypeName;
    qmlSpecifier_ = createObject(getBasicComponentQmlPath("CardSpecifier"+cardSpecifierTypeName), qmlObject_, cardSpecifierTypeName);
    connect(qmlSpecifier_, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged(QString)));
    switch (cardSpecifier.type) {
    case asn::CardSpecifierType::CardType: {
        const auto &value = std::get<asn::CardType>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(value))));
        break;
    }
    case asn::CardSpecifierType::Owner: {
        const auto &value = std::get<asn::Player>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(value))));
        break;
    }
    case asn::CardSpecifierType::Trait: {
        const auto &value = std::get<asn::Trait>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(value.value)));
        break;
    }
    case asn::CardSpecifierType::ExactName: {
        const auto &value = std::get<asn::ExactName>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(value.value)));
        break;
    }
    case asn::CardSpecifierType::NameContains: {
        const auto &value = std::get<asn::NameContains>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(value.value)));
        break;
    }
    case asn::CardSpecifierType::Level: {
        const auto &value = std::get<asn::Level>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setNumMod", Q_ARG(QVariant, QString::fromStdString(toString(value.value.mod))));
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::number(value.value.value)));
        break;
    }
    case asn::CardSpecifierType::Color: {
        const auto &value = std::get<asn::Color>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(value))));
        break;
    }
    case asn::CardSpecifierType::Cost: {
        const auto &value = std::get<asn::CostSpecifier>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setNumMod", Q_ARG(QVariant, QString::fromStdString(toString(value.value.mod))));
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::number(value.value.value)));
        break;
    }
    case asn::CardSpecifierType::TriggerIcon: {
        const auto &value = std::get<asn::TriggerIcon>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(value))));
        break;
    }
    case asn::CardSpecifierType::Power: {
        const auto &value = std::get<asn::Power>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setNumMod", Q_ARG(QVariant, QString::fromStdString(toString(value.value.mod))));
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::number(value.value.value)));
        break;
    }
    case asn::CardSpecifierType::State: {
        const auto &value = std::get<asn::State>(cardSpecifier.specifier);
        QMetaObject::invokeMethod(qmlSpecifier_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(value))));
        break;
    }
    }
}

void CardSpecifierComponent::valueChanged(QString value) {
    switch (specifier_.type) {
    case asn::CardSpecifierType::CardType:
        specifier_.specifier = parse(value.toStdString(), formats::To<asn::CardType>{});
        break;
    case asn::CardSpecifierType::Owner:
        specifier_.specifier = parse(value.toStdString(), formats::To<asn::Player>{});
        break;
    case asn::CardSpecifierType::Trait:
        specifier_.specifier = asn::Trait{.value = value.toStdString()};
        break;
    case asn::CardSpecifierType::ExactName:
        specifier_.specifier = asn::ExactName{.value = value.toStdString()};
        break;
    case asn::CardSpecifierType::NameContains:
        specifier_.specifier = asn::NameContains{.value = value.toStdString()};
        break;
    case asn::CardSpecifierType::Level: {
        auto &specifier = std::get<asn::Level>(specifier_.specifier);
        specifier.value.value = value.toInt();
        break;
    }
    case asn::CardSpecifierType::Color:
        specifier_.specifier = parse(value.toStdString(), formats::To<asn::Color>{});
        break;
    case asn::CardSpecifierType::Cost: {
        auto &specifier = std::get<asn::CostSpecifier>(specifier_.specifier);
        specifier.value.value = value.toInt();
        break;
    }
    case asn::CardSpecifierType::TriggerIcon:
        specifier_.specifier = parse(value.toStdString(), formats::To<asn::TriggerIcon>{});
        break;
    case asn::CardSpecifierType::Power: {
        auto &specifier = std::get<asn::Power>(specifier_.specifier);
        specifier.value.value = value.toInt();
        break;
    }
    case asn::CardSpecifierType::State:
        specifier_.specifier = parse(value.toStdString(), formats::To<asn::State>{});
        break;
    default: throw std::logic_error("unhandled cardSpecifier valueChanged");
    }
    emit componentReady(specifier_, componentId_);
}

void CardSpecifierComponent::numModifierChanged(QString value) {
    switch (specifier_.type) {
    case asn::CardSpecifierType::Level: {
        auto &specifier = std::get<asn::Level>(specifier_.specifier);
        specifier.value.mod = parse(value.toStdString(), formats::To<asn::NumModifier>{});
        break;
    }
    case asn::CardSpecifierType::Cost: {
        auto &specifier = std::get<asn::CostSpecifier>(specifier_.specifier);
        specifier.value.mod = parse(value.toStdString(), formats::To<asn::NumModifier>{});
        break;
    }
    case asn::CardSpecifierType::Power: {
        auto &specifier = std::get<asn::Power>(specifier_.specifier);
        specifier.value.mod = parse(value.toStdString(), formats::To<asn::NumModifier>{});
        break;
    }
    default: throw std::logic_error("unhandled numModifierChanged");
    }
    emit componentReady(specifier_, componentId_);
}
