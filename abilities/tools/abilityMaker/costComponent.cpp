#include "costComponent.h"

#include <QQmlContext>

namespace {
const int kElemOffset = 160;
QQuickItem* createQmlObject(const QString &name, QQuickItem *parent) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    return qmlObject;
}

void initCostItems(asn::CostItem &item) {
    if (item.type == asn::CostType::Stock) {
        item.costItem = asn::StockCost{1};
    } else {
        item.costItem = asn::Effect();
    }
}
}

CostComponent::CostComponent(QQuickItem *parent)
    : BaseComponent("Cost", parent, "cost") {
    init();
}

CostComponent::CostComponent(const asn::Cost &cost, QQuickItem *parent)
    : BaseComponent("Cost", parent, "cost") {
    init();

    if (cost.items.empty())
        return;

    costItems = cost.items;
    initState = true;
    for (int i = 0; i < costItems.size(); ++i) {
        if (costItems[i].type == asn::CostType::Effects)
            effectSet.push_back(true);
        else
            effectSet.push_back(false);
        createCost();
        auto qmlCostItem = qmlCostItems.back().first;

        QMetaObject::invokeMethod(qmlCostItem, "setType",
                                  Q_ARG(QVariant, static_cast<int>(costItems[i].type)));
    }
}

void CostComponent::init() {
    qmlObject->setProperty("kOffset", kElemOffset);
    connect(qmlObject, SIGNAL(addCost()), this, SLOT(addCost()));
}

void CostComponent::addCost() {
    createCost();
    costItems.emplace_back();
    effectSet.push_back(false);
}

void CostComponent::createCost() {
    auto obj = createQmlObject("basicTypes/CostType", qmlObject);
    obj->setProperty("position", QVariant((int)qmlCostItems.size()));
    obj->setProperty("x", QVariant((int)(kElemOffset * qmlCostItems.size() + 10)));
    obj->setProperty("y", 100);
    connect(obj, SIGNAL(valueChanged(int,int)), this, SLOT(onCostTypeChanged(int,int)));
    qmlCostItems.push_back(CostItem{obj, nullptr});
    qmlObject->setProperty("costCount", qmlObject->property("costCount").toInt() + 1);
}

void CostComponent::onCostTypeChanged(int pos, int value) {
    if (qmlCostItems[pos].second) {
        qmlCostItems[pos].second->deleteLater();
        qmlCostItems[pos].second = nullptr;
    }

    auto costType = static_cast<asn::CostType>(value);
    costItems[pos].type = costType;
    if (!initState)
        initCostItems(costItems[pos]);
    QQuickItem *obj;
    switch (costType) {
    case asn::CostType::Stock: {
        obj = createQmlObject("basicTypes/CardSpecifierTextInput", qmlObject);
        if (initState)
            initComponent(pos, obj);
        else
            QMetaObject::invokeMethod(obj, "setValue", Q_ARG(QVariant, "1"));
        connect(obj, SIGNAL(valueChanged(int,QString)), this, SLOT(stringSet(int,QString)));
        effectSet[pos] = false;
        break;
    }
    case asn::CostType::Effects: {
        obj = createQmlObject("basicTypes/StockCostEffects", qmlObject);
        connect(obj, SIGNAL(editEffect(int)), this, SLOT(editEffect(int)));
        break;
    }
    default:
        return;
    }

    qmlCostItems[pos].second = obj;
    obj->setProperty("position", pos);
    obj->setProperty("x", qmlCostItems[pos].first->property("x").toDouble());
    obj->setProperty("y", qmlCostItems[pos].first->property("y").toDouble() + 10 +
                          qmlCostItems[pos].first->property("height").toDouble());

    // End component initialization
    if (pos == costItems.size() - 1)
        initState = false;
}

void CostComponent::stringSet(int pos, QString value) {
    switch (costItems[pos].type) {
    case asn::CostType::Stock:
        costItems[pos].costItem = asn::StockCost{ value.toInt() };
        break;
    default:
        assert(false);
    }
}

void CostComponent::editEffect(int pos) {
    if (effectSet[pos]) {
        auto &ef = std::get<asn::Effect>(costItems[pos].costItem);
        qmlEffect = std::make_unique<EffectComponent>(ef, qmlObject, pos);
    } else
        qmlEffect = std::make_unique<EffectComponent>(qmlObject, pos);

    currentPos = pos;
    connect(qmlEffect.get(), &EffectComponent::componentChanged, this, &CostComponent::effectReady);
    connect(qmlEffect.get(), &EffectComponent::close, this, &CostComponent::destroyEffect);
}

void CostComponent::destroyEffect() {
    qmlEffect.reset();
}

void CostComponent::effectReady(const asn::Effect &effect) {
    effectSet[currentPos] = true;

    auto dup = effect;
    asn::Condition cond;
    cond.type = asn::ConditionType::NoCondition;
    dup.cond = cond;
    costItems[currentPos].costItem = dup;

    emit componentChanged(constructCost());
}

void CostComponent::componentReady() {
    emit componentChanged(constructCost());
    emit close();
}

std::optional<asn::Cost> CostComponent::constructCost() {
    std::erase_if(costItems, [](const auto &spec){ return static_cast<int>(spec.type) == 0; });

    std::optional<asn::Cost> optCost;
    if (costItems.empty())
        return optCost;

    asn::Cost cost;
    cost.items = costItems;
    optCost = cost;
    return optCost;
}

namespace {
template<typename T>
void initComponentNum(QQuickItem *obj, const asn::CostItem &item) {
    auto &num = std::get<T>(item.costItem).value;
    QMetaObject::invokeMethod(obj, "setValue", Q_ARG(QVariant, QString::number(num)));
}
}

void CostComponent::initComponent(int pos, QQuickItem *obj) {
    switch (costItems[pos].type) {
    case asn::CostType::Stock:
        initComponentNum<asn::StockCost>(obj, costItems[pos]);
        break;
    case asn::CostType::Effects:
        break;
    }
}
