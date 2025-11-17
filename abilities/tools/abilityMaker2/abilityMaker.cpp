#include "abilityMaker.h"

#include <QQmlContext>

#include "trigger.h"
#include "triggerInit.h"
#include "language_parser.h"

void AbilityMaker::componentComplete() {
    QQuickItem::componentComplete();
}

QString AbilityMaker::translate(const asn::Ability &ability) {
    return QString::fromStdString(printAbility(ability));
}
