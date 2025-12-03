#include "trigger.h"

#include <set>

#include <QQmlContext>

#include "componentHelper.h"
#include "languageSpecification.h"
#include "language_parser.h"
#include "triggerInit.h"


TriggerComponent::TriggerComponent(QQuickItem *parent)
    : BaseComponent("Trigger", parent) {
    init(parent);
}

TriggerComponent::TriggerComponent(QQuickItem *parent, const std::vector<asn::Trigger>& triggers)
    : BaseComponent("Trigger", parent) {
    init(parent);
    type_ = triggers.at(0).type;
    trigger_ = triggers.at(0).trigger;
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createTrigger();
}

void TriggerComponent::init(QQuickItem *parent) {
    qvariant_cast<QObject*>(qmlObject_->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    connect(qmlObject_, SIGNAL(triggerTypeChanged(QString)), this, SLOT(onTriggerTypeChanged(QString)));
}

TriggerComponent::~TriggerComponent() {}

void TriggerComponent::fitComponent(QQuickItem* object) {
    if (components_.empty()) {
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", qmlObject_->property("left"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(50));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", qmlObject_->property("top"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("topMargin", QVariant(130));
    } else {
        auto *lastObject = components_.back();
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("left", lastObject->property("right"));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("leftMargin", QVariant(10));
        qvariant_cast<QObject*>(object->property("anchors"))->setProperty("top", lastObject->property("top"));
    }
    components_.push_back(object);
}

void TriggerComponent::onTriggerTypeChanged(QString type) {
    qDebug() << "changing trigger type";
    type_ = parse(type.toStdString(), formats::To<asn::TriggerType>{});
    trigger_ = getDefaultTrigger(type_);
    createTrigger();
}


void TriggerComponent::createTrigger() {
    components_.clear();
    componentManager_.clear();
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponents(QString::fromStdString(toString(type_)));
    std::set<std::string> types;
    for (const auto& comp: components) {
        QString component_id = comp.type + (types.contains(comp.type.toStdString()) ? QString("2") : QString(""));
        types.insert(comp.type.toStdString());
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject_, this);
        fitComponent(object);
    }

    setTriggerInQml();
    notifyOfChanges();
}

void TriggerComponent::notifyOfChanges() {
    emit componentChanged(constructTrigger());
}

std::vector<asn::Trigger> TriggerComponent::constructTrigger() {
    std::vector<asn::Trigger> vec;
    vec.push_back(asn::Trigger{.type=type_,.trigger=trigger_});
    return vec;
}
