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
    setCondition(condition);
    qvariant_cast<QObject*>(qmlObject_->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
}

void ConditionComponent::init(QQuickItem *parent) {
    gen_helper = std::make_shared<gen::ConditionHelper>(this);
    connect(qmlObject_, SIGNAL(conditionTypeChanged(QString)), this, SLOT(onConditionTypeChanged(QString)));
}

void ConditionComponent::setCondition(asn::Condition condition) {
    type_ = condition.type;
    condition_ = condition.cond;
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createCondition();
}

void ConditionComponent::onConditionTypeChanged(QString type) {
    type_ = parse(type.toStdString(), formats::To<asn::ConditionType>{});
    condition_ = getDefaultCondition(type_);
    createCondition();
    notifyOfChanges();
}

void ConditionComponent::fitComponent() {
    auto bottomObject = qmlObject_;
    QQuickItem *lastObject, *currentLastObject;
    for (int x = 0; x < components_.size(); ++x) {
        for (int y = 0; y < components_[x].size(); ++y) {
            if (x == 0) {
                qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("left", qmlObject_->property("left"));
                qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("leftMargin", QVariant(50));
                if (y == 0) {
                    qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("top", qmlObject_->property("top"));
                    qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("topMargin", QVariant(130));
                    lastObject = components_[x][y];
                    currentLastObject = components_[x][y];
                } else {
                    qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("top", bottomObject->property("bottom"));
                    qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("topMargin", QVariant(10));
                }
                bottomObject = components_[x][y];
            } else {
                qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("left", lastObject->property("right"));
                qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("leftMargin", QVariant(10));
                if (y == 0) {
                    qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("top", lastObject->property("top"));
                    currentLastObject = components_[x][y];
                } else {
                    qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("top", bottomObject->property("bottom"));
                    qvariant_cast<QObject*>(components_[x][y]->property("anchors"))->setProperty("topMargin", QVariant(10));
                }
                bottomObject = components_[x][y];
            }
        }
        lastObject = currentLastObject;
    }
}

void ConditionComponent::createCondition() {
    components_.clear();
    componentManager_.clear();
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByEnum(QString::fromStdString(toString(type_)));
    int i{0};
    for (const auto& comp: components) {
        size_t arraySize = gen_helper->getArraySize(type_, condition_, comp.name);
        auto objects = componentManager_.createComponent(comp, qmlObject_, this, gen_helper.get(), ++i, arraySize);
        if (!objects.size())
            continue;
        components_.push_back(objects);
        fitComponent();
        if (comp.type == "Condition") {
            for (auto object: objects) {
                object->setProperty("scale", QVariant(0.8));
            }
        }
    }

    gen_helper->setConditionInQml(type_, condition_);
}

void ConditionComponent::notifyOfChanges() {
    emit componentChanged(componentId_, type_, condition_);
    emit conditionReady(asn::Condition{.type=type_,.cond=condition_}, componentId_);
    qreal width{70}, height{0};
    for (const auto& componentsRow: components_) {
        auto it = std::max_element(componentsRow.begin(), componentsRow.end(), [](QQuickItem* a, QQuickItem* b){ return a->width() < b->width(); });
        width += (*it)->width() + 10;
        height = std::max(height, std::accumulate(componentsRow.begin(), componentsRow.end(), qreal{0}, [](qreal total, QQuickItem* next) { return total + next->height(); }) + 180);
    }
    emit sizeChanged(width, height+20);
}

void ConditionComponent::addComponentToArray(QString type, QString fieldName, int typePosition) {
    gen_helper->addElementToArray(fieldName);
    auto updatedComponents = componentManager_.getComponentsRow(type, typePosition);
    if (type == "Condition") {
        for (auto object: updatedComponents) {
            object->setProperty("scale", QVariant(0.8));
        }
    }
    components_[typePosition-1] = updatedComponents;
    fitComponent();
    notifyOfChanges();
}

void ConditionComponent::removeComponentFromArray(QString type, QString fieldName, int typePosition) {
    gen_helper->removeElementFromArray(fieldName);
    auto updatedComponents = componentManager_.getComponentsRow(type, typePosition);
    components_[typePosition-1] = updatedComponents;
    fitComponent();
    notifyOfChanges();
}
