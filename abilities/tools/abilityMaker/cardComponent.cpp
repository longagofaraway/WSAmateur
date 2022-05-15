#include "cardComponent.h"

#include "multiplierComponent.h"

#include <QQmlContext>

namespace {
int kElemOffset = 160;
QQuickItem* createQmlObject(const QString &name, QQuickItem *parent) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    return qmlObject;
}

void initSpecifier(asn::CardSpecifier &spec) {
    auto defaultNumber = asn::Number{ asn::NumModifier::ExactMatch, 0 };
    switch (spec.type) {
    case asn::CardSpecifierType::CardType:
        spec.specifier = asn::CardType::Char;
        break;
    case asn::CardSpecifierType::Owner:
        spec.specifier = asn::Player::Player;
        break;
    case asn::CardSpecifierType::Color:
        spec.specifier = asn::Color::Yellow;
        break;
    case asn::CardSpecifierType::TriggerIcon:
        spec.specifier = asn::TriggerIcon::Soul;
        break;
    case asn::CardSpecifierType::Level:
        spec.specifier = asn::Level{ defaultNumber };
        break;
    case asn::CardSpecifierType::Cost:
        spec.specifier = asn::CostSpecifier{ defaultNumber };
        break;
    case asn::CardSpecifierType::Power:
        spec.specifier = asn::Power{ defaultNumber };
        break;
    case asn::CardSpecifierType::LevelWithMultiplier: {
        auto sp = asn::LevelWithMultiplier();
        sp.value = defaultNumber;
        sp.multiplier.type = asn::MultiplierType::TimesLevel;
        spec.specifier = sp;
        break;
    }
    case asn::CardSpecifierType::State:
        spec.specifier = asn::State::Standing;
        break;
    default:
        break;
    }
}

const std::optional<asn::Multiplier> getMultiplier(asn::CardSpecifier &specifier) {
    switch (specifier.type) {
    case asn::CardSpecifierType::LevelWithMultiplier: {
        const auto &e = std::get<asn::LevelWithMultiplier>(specifier.specifier);
        return e.multiplier;
    }
    default:
        assert(false);
    }
    throw std::runtime_error("unhandled card specifier in multiplier");
}
}

CardComponent::CardComponent(QQuickItem *parent)
    : BaseComponent("Card", parent, "card") {
    init();
}

CardComponent::CardComponent(const asn::Card &card, QQuickItem *parent)
    : BaseComponent("Card", parent, "card") {
    init();

    if (card.cardSpecifiers.empty())
        return;

    specifiers = card.cardSpecifiers;
    initState = true;
    for (int i = 0; i < specifiers.size(); ++i) {
        createSpecifier();
        auto qmlSpecifier = qmlSpecifiers.back().first;

        QMetaObject::invokeMethod(qmlSpecifier, "setType",
                                  Q_ARG(QVariant, static_cast<int>(specifiers[i].type)));
    }
}

void CardComponent::moveToTop() {
    // set y after setting the parent
    QMetaObject::invokeMethod(qmlObject, "setActualY");
}

void CardComponent::addSpecifier() {
    createSpecifier();
    specifiers.emplace_back();
}

void CardComponent::onSpecifierChanged(int pos, int value) {
    if (qmlSpecifiers[pos].second) {
        qmlSpecifiers[pos].second->deleteLater();
        qmlSpecifiers[pos].second = nullptr;
    }

    auto specType = static_cast<asn::CardSpecifierType>(value);
    specifiers[pos].type = specType;
    if (!initState)
        initSpecifier(specifiers[pos]);
    QQuickItem *obj;
    switch (specType) {
    case asn::CardSpecifierType::CardType: {
        obj = createQmlObject("basicTypes/CardType", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(valueChanged(int,int)), this, SLOT(enumSet(int,int)));
        break;
    }
    case asn::CardSpecifierType::Owner: {
        obj = createQmlObject("basicTypes/CardSpecifierPlayer", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(valueChangedEx(int,int)), this, SLOT(enumSet(int,int)));
        break;
    }
    case asn::CardSpecifierType::Color: {
        obj = createQmlObject("basicTypes/Color", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(valueChanged(int,int)), this, SLOT(enumSet(int,int)));
        break;
    }
    case asn::CardSpecifierType::TriggerIcon: {
        obj = createQmlObject("basicTypes/TriggerIcon", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(valueChanged(int,int)), this, SLOT(enumSet(int,int)));
        break;
    }
    case asn::CardSpecifierType::Trait:
    case asn::CardSpecifierType::ExactName:
    case asn::CardSpecifierType::NameContains: {
        obj = createQmlObject("basicTypes/CardSpecifierTextInput", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(valueChanged(int,QString)), this, SLOT(stringSet(int,QString)));
        break;
    }
    case asn::CardSpecifierType::Level:
    case asn::CardSpecifierType::Cost:
    case asn::CardSpecifierType::Power: {
        obj = createQmlObject("basicTypes/CardSpecifierNumber", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(numModifierChangedEx(int,int)), this, SLOT(enumSet(int,int)));
        connect(obj, SIGNAL(valueChangedEx(int,QString)), this, SLOT(stringSet(int,QString)));
        break;
    }
    case asn::CardSpecifierType::LevelWithMultiplier: {
        obj = createQmlObject("basicTypes/CardSpecifierNumberWithMultiplier", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(numModifierChangedEx(int,int)), this, SLOT(enumSet(int,int)));
        connect(obj, SIGNAL(valueChangedEx(int,QString)), this, SLOT(stringSet(int,QString)));
        connect(obj, SIGNAL(editMultiplier(int)), this, SLOT(editMultiplier(int)));
        break;
    }
    case asn::CardSpecifierType::State: {
        obj = createQmlObject("basicTypes/CardSpecifierState", qmlObject);
        if (initState)
            initComponent(pos, obj);
        connect(obj, SIGNAL(valueChangedEx(int,int)), this, SLOT(enumSet(int,int)));
        break;
    }
    default:
        return;
    }

    qmlSpecifiers[pos].second = obj;
    obj->setProperty("position", pos);
    obj->setProperty("x", qmlSpecifiers[pos].first->property("x").toDouble());
    obj->setProperty("y", qmlSpecifiers[pos].first->property("y").toDouble() + 10 +
                          qmlSpecifiers[pos].first->property("height").toDouble());

    // End component initialization
    if (pos == specifiers.size() - 1)
        initState = false;
}

namespace {
template<typename T>
void setNumModifier(asn::CardSpecifier &spec, int value) {
    auto &number = std::get<T>(spec.specifier);
    number.value.mod = static_cast<asn::NumModifier>(value);
}

template<typename T>
void setNumValue(asn::CardSpecifier &spec, const QString &value) {
    auto &number = std::get<T>(spec.specifier);
    number.value.value = value.toInt();
}
}

void CardComponent::enumSet(int pos, int value) {
    switch (specifiers[pos].type) {
    case asn::CardSpecifierType::CardType:
        specifiers[pos].specifier = static_cast<asn::CardType>(value);
        break;
    case asn::CardSpecifierType::Owner:
        specifiers[pos].specifier = static_cast<asn::Player>(value);
        break;
    case asn::CardSpecifierType::Color:
        specifiers[pos].specifier = static_cast<asn::Color>(value);
        break;
    case asn::CardSpecifierType::TriggerIcon:
        specifiers[pos].specifier = static_cast<asn::TriggerIcon>(value);
        break;
    case asn::CardSpecifierType::Level:
        setNumModifier<asn::Level>(specifiers[pos], value);
        break;
    case asn::CardSpecifierType::Cost:
        setNumModifier<asn::CostSpecifier>(specifiers[pos], value);
        break;
    case asn::CardSpecifierType::Power:
        setNumModifier<asn::Power>(specifiers[pos], value);
        break;
    case asn::CardSpecifierType::LevelWithMultiplier:
        setNumModifier<asn::LevelWithMultiplier>(specifiers[pos], value);
        break;
    case asn::CardSpecifierType::State:
        specifiers[pos].specifier = static_cast<asn::State>(value);
        break;
    default:
        assert(false);
    }
}

void CardComponent::stringSet(int pos, QString value) {
    switch (specifiers[pos].type) {
    case asn::CardSpecifierType::Trait:
        specifiers[pos].specifier = asn::Trait{ value.toStdString() };
        break;
    case asn::CardSpecifierType::ExactName:
        specifiers[pos].specifier = asn::ExactName{ value.toStdString() };
        break;
    case asn::CardSpecifierType::NameContains:
        specifiers[pos].specifier = asn::NameContains{ value.toStdString() };
        break;
    case asn::CardSpecifierType::Level:
        setNumValue<asn::Level>(specifiers[pos], value);
        break;
    case asn::CardSpecifierType::Cost:
        setNumValue<asn::CostSpecifier>(specifiers[pos], value);
        break;
    case asn::CardSpecifierType::Power:
        setNumValue<asn::Power>(specifiers[pos], value);
        break;
    case asn::CardSpecifierType::LevelWithMultiplier:
        setNumValue<asn::LevelWithMultiplier>(specifiers[pos], value);
        break;
    default:
        assert(false);
    }
}

void CardComponent::editMultiplier(int pos) {
    const auto &m = getMultiplier(specifiers[pos]);
    currentPos = pos;
    qmlMultiplier = std::make_shared<MultiplierComponent>(*m, qmlObject);

    connect(qmlMultiplier.get(), &MultiplierComponent::componentChanged, this, &CardComponent::multiplierReady);
    connect(qmlMultiplier.get(), &MultiplierComponent::close, this, &CardComponent::destroyMultiplier);
}

void CardComponent::destroyMultiplier() {
    qmlMultiplier.reset();
}

void CardComponent::multiplierReady(const asn::Multiplier &m) {
    auto &sp = specifiers[currentPos];
    switch (sp.type) {
    case asn::CardSpecifierType::LevelWithMultiplier: {
        auto &e = std::get<asn::LevelWithMultiplier>(sp.specifier);
        e.multiplier = m;
        break;
    }
    default:
        assert(false);
    }
}

void CardComponent::createSpecifier() {
    auto obj = createQmlObject("basicTypes/CardSpecifierType", qmlObject);
    obj->setProperty("position", QVariant((int)qmlSpecifiers.size()));
    obj->setProperty("x", QVariant((int)(kElemOffset * qmlSpecifiers.size() + 10)));
    obj->setProperty("y", 100);
    connect(obj, SIGNAL(valueChanged(int,int)), this, SLOT(onSpecifierChanged(int,int)));
    qmlSpecifiers.push_back(CardSpecifier{obj, nullptr});
    qmlObject->setProperty("specCount", qmlObject->property("specCount").toInt() + 1);
}

void CardComponent::init() {
    qmlObject->setProperty("kOffset", kElemOffset);
    connect(qmlObject, SIGNAL(addSpecifier()), this, SLOT(addSpecifier()));
}

void CardComponent::componentReady() {
    emit componentChanged(constructCard());
    emit close();
}

asn::Card CardComponent::constructCard() {
    std::erase_if(specifiers, [](const auto &spec){ return static_cast<int>(spec.type) == 0; });

    asn::Card card;
    card.cardSpecifiers = specifiers;
    return card;
}

namespace {
template<typename T>
void initComponentEnum(QQuickItem *obj, const asn::CardSpecifier &spec) {
    int value = static_cast<int>(std::get<T>(spec.specifier));
    if constexpr (std::is_same_v<T, asn::Player> || std::is_same_v<T, asn::State>)
        QMetaObject::invokeMethod(obj, "setValueEx", Q_ARG(QVariant, value));
    else
        QMetaObject::invokeMethod(obj, "setValue", Q_ARG(QVariant, value));
}
template<typename T>
void initComponentNum(QQuickItem *obj, const asn::CardSpecifier &spec) {
    auto &num = std::get<T>(spec.specifier).value;
    QMetaObject::invokeMethod(obj, "setValue", Q_ARG(QVariant, QString::number(num.value)));
    QMetaObject::invokeMethod(obj, "setNumMod", Q_ARG(QVariant, static_cast<int>(num.mod)));
}
template<typename T>
void initComponentString(QQuickItem *obj, const asn::CardSpecifier &spec) {
    auto num = std::get<T>(spec.specifier);
    QMetaObject::invokeMethod(obj, "setValue", Q_ARG(QVariant, QString::fromStdString(num.value)));
}
}

void CardComponent::initComponent(int pos, QQuickItem *obj) {
    switch (specifiers[pos].type) {
    case asn::CardSpecifierType::CardType:
        initComponentEnum<asn::CardType>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::Owner:
        initComponentEnum<asn::Player>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::Color:
        initComponentEnum<asn::Color>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::TriggerIcon:
        initComponentEnum<asn::TriggerIcon>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::Level:
        initComponentNum<asn::Level>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::Cost:
        initComponentNum<asn::CostSpecifier>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::Power:
        initComponentNum<asn::Power>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::Trait:
        initComponentString<asn::Trait>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::ExactName:
        initComponentString<asn::ExactName>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::NameContains:
        initComponentString<asn::NameContains>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::LevelWithMultiplier:
        initComponentNum<asn::LevelWithMultiplier>(obj, specifiers[pos]);
        break;
    case asn::CardSpecifierType::State:
        initComponentEnum<asn::State>(obj, specifiers[pos]);
        break;
    default:
        break;
    }
}
