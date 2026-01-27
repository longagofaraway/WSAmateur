#include "condition.h"

#include <set>

#include "ability_maker_gen.h"
#include "languageSpecification.h"
#include "language_parser.h"
#include "conditionInit.h"

ConditionComponent::ConditionComponent(QString nodeId, QQuickItem *parent)
    : BaseComponent("Condition", parent, nodeId) {
    init(parent);
}

ConditionComponent::ConditionComponent(QString nodeId, QQuickItem *parent, const asn::Condition& condition)
    : BaseComponent("Condition", parent, nodeId) {
    init(parent);
    type_ = condition.type;
    condition_ = condition.cond;
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createCondition();
}

void ConditionComponent::init(QQuickItem *parent) {
    gen_helper = std::make_shared<gen::ConditionHelper>(this);
    qvariant_cast<QObject*>(qmlObject_->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    connect(qmlObject_, SIGNAL(conditionTypeChanged(QString)), this, SLOT(onConditionTypeChanged(QString)));
}

void ConditionComponent::onConditionTypeChanged(QString type) {
    type_ = parse(type.toStdString(), formats::To<asn::ConditionType>{});
    condition_ = getDefaultCondition(type_);
    createCondition();
    notifyOfChanges();
}

void ConditionComponent::fitComponent(QQuickItem* object) {
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

void ConditionComponent::createCondition() {
    components_.clear();
    componentManager_.clear();
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByEnum(QString::fromStdString(toString(type_)));
    std::set<std::string> types;
    for (const auto& comp: components) {
        if (comp.type == "Condition") {
            continue;
        }
        QString component_id = comp.type + "/" + (types.contains(comp.type.toStdString()) ? QString("2") : QString(""));
        types.insert(comp.type.toStdString());
        auto *object = componentManager_.createComponent(comp.type, comp.name, component_id, qmlObject_, this, gen_helper.get());
        fitComponent(object);
    }

    gen_helper->setConditionInQml(type_, condition_);
}

void ConditionComponent::notifyOfChanges() {
    emit componentChanged(componentId_, type_, condition_);
    qreal width{70}, height{0};
    for (const auto component: components_) {
        width += component->width() + 10;
        height = std::max(height, component->height());
    }
    emit sizeChanged(width, height);
}
