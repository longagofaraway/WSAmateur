#include "componentHelper.h"

#include <stdexcept>

#include "abilities.h"

QString getBasicComponentQmlPath(QString componentName) {
    if (componentName == "Zone") {
        return "BasicTypes/Zone";
    }
    if (componentName == "Phase") {
        return "BasicTypes/Phase";
    }
    if (componentName == "Player") {
        return "BasicTypes/Player";
    }
    if (componentName == "State") {
        return "BasicTypes/State";
    }
    if (componentName == "AttackType") {
        return "BasicTypes/AttackType";
    }
    if (componentName == "AbilityType") {
        return "BasicTypes/AbilityType";
    }
    if (componentName == "Target") {
        return "Target";
    }
    if (componentName == "Bool") {
        return "BasicTypes/BoolCheckBox";
    }

    if (componentName == "CardSpecifierCardType") {
        return "BasicTypes/CardSpecifierCardType";
    }
    if (componentName == "CardSpecifierOwner") {
        return "BasicTypes/CardSpecifierOwner";
    }
    if (componentName == "CardSpecifierState") {
        return "BasicTypes/CardSpecifierState";
    }
    if (componentName == "CardSpecifierTrait" ||
            componentName == "CardSpecifierExactName" ||
            componentName == "CardSpecifierNameContains") {
        return "BasicTypes/CardSpecifierTextInput";
    }
    if (componentName == "CardSpecifierLevel" ||
            componentName == "CardSpecifierCost" ||
            componentName == "CardSpecifierPower") {
        return "BasicTypes/CardSpecifierNumber";
    }
    throw std::runtime_error("unimplemented componentName");
}

/*void connectComponent(QString componentName, QQuickItem *qmlObject, QObject* object) {
    if (componentName == "Target") {
        QObject::connect(qmlObject, SIGNAL(componentChanged(QString), object, SLOT(targetChanged())));
    }
}*/
