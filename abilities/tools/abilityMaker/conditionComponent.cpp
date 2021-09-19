#include "conditionComponent.h"

ConditionComponent::ConditionComponent(QQuickItem *parent)
    : BaseComponent("Condition", parent) {
    init();
}

ConditionComponent::ConditionComponent(const asn::Condition &c, QQuickItem *parent)
    : BaseComponent("Condition", parent) {
    init();
    initCondition(c);
}

void ConditionComponent::init() {
    connect(qmlObject, SIGNAL(conditionTypeChanged(int)), this, SLOT(onConditionTypeChanged(int)));

    connect(this, SIGNAL(passConditionType(int)), qmlObject, SIGNAL(incomingConditionType(int)));
}

void ConditionComponent::initCondition(const asn::Condition &c) {
    initializing = true;
    type = c.type;
    condition = c.cond;
    emit passConditionType((int)type);
}

void ConditionComponent::componentReady() {
    emit componentChanged(constructCondition());
    emit close();
}

asn::Condition ConditionComponent::constructCondition() {
    asn::Condition c;
    c.type = type;
    c.cond = condition;
    return c;
}

void ConditionComponent::onConditionTypeChanged(int index) {
    bool needImpl = true;
    type = static_cast<asn::ConditionType>(index);

    if (initializing) {
        if (needImpl)
            qmlConditionImpl = std::make_unique<ConditionImplComponent>(type, condition, qmlObject);
        else
            qmlConditionImpl.reset();
        initializing = false;
        emit componentChanged(constructCondition());
    } else {
        type = static_cast<asn::ConditionType>(index);

        if (needImpl) {
            qmlConditionImpl = std::make_unique<ConditionImplComponent>(type, qmlObject);
            initConditionByType(condition, type);
        } else {
            qmlConditionImpl.reset();
        }
    }

    if (needImpl)
        connect(qmlConditionImpl.get(), &ConditionImplComponent::componentChanged, this, &ConditionComponent::onConditionChanged);
}

void ConditionComponent::onConditionChanged(const VarCondition &c) {
    condition = c;
    emit componentChanged(constructCondition());
}
