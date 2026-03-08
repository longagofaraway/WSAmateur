#include "trigger.h"

#include <set>

#include <QQmlContext>

#include "ability_maker_gen.h"
#include "componentHelper.h"
#include "languageSpecification.h"
#include "language_parser.h"
#include "triggerInit.h"


TriggerComponent::TriggerComponent(QQuickItem *parent)
    : BaseComponent("Trigger", parent, "Trigger") {
    init(parent);
}

TriggerComponent::TriggerComponent(QQuickItem *parent, const std::vector<asn::Trigger>& triggers)
    : BaseComponent("Trigger", parent, "Trigger") {
    init(parent);
    type_ = triggers.at(0).type;
    trigger_ = triggers.at(0).trigger;
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createTrigger();
}

void TriggerComponent::init(QQuickItem *parent) {
    gen_helper = std::make_shared<gen::TriggerHelper>(this);
    qvariant_cast<QObject*>(qmlObject_->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    connect(qmlObject_, SIGNAL(triggerTypeChanged(QString)), this, SLOT(onTriggerTypeChanged(QString)));
}

TriggerComponent::~TriggerComponent() {}

void TriggerComponent::fitComponent() {
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
    auto components = spec.getComponentsByEnum(QString::fromStdString(toString(type_)));
    int i{0};
    for (const auto& comp: components) {
        size_t arraySize = gen_helper->getArraySize(type_, trigger_, comp.name);
        auto objects = componentManager_.createComponent(comp, qmlObject_, this, gen_helper.get(), ++i, arraySize);
        components_.push_back(objects);
        fitComponent();
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

void TriggerComponent::addComponentToArray(QString type, QString fieldName, int typePosition) {
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
