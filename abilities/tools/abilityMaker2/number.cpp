#include "number.h"

#include "language_parser.h"
#include "languageSpecification.h"

NumberComponent::NumberComponent(QQuickItem *parent, QString id, QString displayName)
    : BaseComponent("BasicTypes/Number", parent, id) {
    qmlObject_->setProperty("displayName", displayName);
    connect(qmlObject_, SIGNAL(numModifierChanged(QString)), this,  SLOT(numModifierChanged(QString)));
    connect(qmlObject_, SIGNAL(valueChanged(QString)), this,  SLOT(valueChanged(QString)));
}

void NumberComponent::setNumber(asn::Number number) {
    number_ = number;

    QMetaObject::invokeMethod(qmlObject_, "setNumMod", Q_ARG(QVariant, QString::fromStdString(toString(number_.mod))));
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::number(number_.value)));
}

void NumberComponent::notifyOfChanges() {
    emit numberReady(number_, componentId_);
}

void NumberComponent::numModifierChanged(QString numModifier) {
    number_.mod = parse(numModifier.toStdString(), formats::To<asn::NumModifier>{});
    notifyOfChanges();
}

void NumberComponent::valueChanged(QString value) {
    number_.value = value.toInt();
    notifyOfChanges();
}
