#include "effect.h"

#include <set>

#include "ability_maker_gen.h"
#include "languageSpecification.h"
#include "language_parser.h"
#include "effectInit.h"

EffectComponent::EffectComponent(QString nodeId, QQuickItem *parent)
    : BaseComponent("Effect", parent, nodeId) {
    init(parent);
}

EffectComponent::EffectComponent(QString nodeId, QQuickItem *parent, const asn::Effect& effect)
    : BaseComponent("Effect", parent, nodeId) {
    init(parent);
    type_ = effect.type;
    effect_ = effect.effect;
    QMetaObject::invokeMethod(qmlObject_, "setValue", Q_ARG(QVariant, QString::fromStdString(toString(type_))));
    createEffect();
}

void EffectComponent::init(QQuickItem *parent) {
    gen_helper = std::make_shared<gen::EffectHelper>(this);
    qvariant_cast<QObject*>(qmlObject_->property("anchors"))->setProperty("fill", QVariant::fromValue(parent));
    connect(qmlObject_, SIGNAL(effectTypeChanged(QString)), this, SLOT(onEffectTypeChanged(QString)));
}

EffectComponent::~EffectComponent() {}

void EffectComponent::fitComponent() {
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

void EffectComponent::createEffect() {
    components_.clear();
    componentManager_.clear();
    auto &spec = LanguageSpecification::get();
    auto components = spec.getComponentsByEnum(QString::fromStdString(toString(type_)));
    int i{0};
    for (const auto& comp: components) {
        if (comp.type == "Effect") {
            continue;
        }
        size_t arraySize = gen_helper->getArraySize(type_, effect_, comp.name);
        auto objects = componentManager_.createComponent(comp, qmlObject_, this, gen_helper.get(), ++i, arraySize);
        if (!objects.size())
            continue;
        components_.push_back(objects);
        fitComponent();
    }

    gen_helper->setEffectInQml(type_, effect_);
}

void EffectComponent::onEffectTypeChanged(QString type) {
    type_ = parse(type.toStdString(), formats::To<asn::EffectType>{});
    effect_ = getDefaultEffect(type_);
    createEffect();
    notifyOfChanges();
}

void EffectComponent::notifyOfChanges() {
    emit componentChanged(componentId_, type_, nullifyOptionalFields(type_, effect_));
    qreal width{70}, height{0};
    for (const auto &componentsRow: components_) {
        auto it = std::max_element(componentsRow.begin(), componentsRow.end(), [](QQuickItem* a, QQuickItem* b){ return a->width() < b->width(); });
        width += (*it)->width() + 10;
        height = std::max(height, std::accumulate(componentsRow.begin(), componentsRow.end(), qreal{0}, [](qreal total, QQuickItem* next) { return total + next->height(); }) + 180);
    }
    emit sizeChanged(width, height);
}

void EffectComponent::addComponentToArray(QString type, QString fieldName, int typePosition) {
    gen_helper->addElementToArray(fieldName);
    auto updatedComponents = componentManager_.getComponentsRow(type, typePosition);
    components_[typePosition-1] = updatedComponents;
    fitComponent();
    notifyOfChanges();
}
