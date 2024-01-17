#include "abilityMaker.h"

#include <QObject>

#include "abilities.h"
#include "abilityComponent.h"
#include "statusLine.h"


void AbilityMaker::componentComplete() {
    QQuickItem::componentComplete();

    initStatusLine(this);
    qmlAbility = std::make_unique<AbilityComponent>(this, -1);
    connect(qmlAbility.get(), &AbilityComponent::componentChanged, this, &AbilityMaker::translate);
    qmlAbility->removeButtons();
    qmlAbility->addDbControls(this);
    abilityPath = "/ability";
    emit updateStatusLine(abilityPath);
}

AbilityMaker::~AbilityMaker() {
    deinitStatusLine();
}

void AbilityMaker::setAbility(const asn::Ability &a) {
    ability_ = a;
    qmlAbility->setAbility(a);

    auto str = printAbility(a);
    auto descr = QString::fromStdString(printAbility(a));
    QMetaObject::invokeMethod(this, "setDescription", Q_ARG(QVariant, descr));
}

void AbilityMaker::statusLinePush(QString dir) {
    abilityPath += "/" + dir;
    emit updateStatusLine(abilityPath);
}

void AbilityMaker::statusLinePop() {
    abilityPath.chop(abilityPath.size() - abilityPath.lastIndexOf('/'));
    emit updateStatusLine(abilityPath);
}

void AbilityMaker::translate(const asn::Ability &ability) {
    ability_ = ability;
    auto str = printAbility(ability);
    auto descr = QString::fromStdString(printAbility(ability));
    QMetaObject::invokeMethod(this, "setDescription", Q_ARG(QVariant, descr));
}
