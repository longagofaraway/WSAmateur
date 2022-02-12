#include "abilityMaker.h"

#include <QObject>

#include "abilities.h"
#include "abilityComponent.h"


void AbilityMaker::componentComplete() {
    QQuickItem::componentComplete();

    qmlAbility = std::make_unique<AbilityComponent>(this);
    connect(qmlAbility.get(), &AbilityComponent::componentChanged, this, &AbilityMaker::translate);
    qmlAbility->removeButtons();
    qmlAbility->addDbControls(this);
}

void AbilityMaker::translate(const asn::Ability &ability) {
    ability_ = ability;
    auto str = printAbility(ability);
    auto descr = QString::fromStdString(printAbility(ability));
    QMetaObject::invokeMethod(this, "setDescription", Q_ARG(QVariant, descr));
}
