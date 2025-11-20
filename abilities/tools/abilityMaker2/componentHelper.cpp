#include "componentHelper.h"

#include <stdexcept>

#include "abilities.h"

QString getBasicComponentQmlPath(QString componentTypeName) {
    if (componentTypeName == "Zone") {
        return "BasicTypes/Zone";
    }
    if (componentTypeName == "Phase") {
        return "BasicTypes/Phase";
    }
    if (componentTypeName == "Player") {
        return "BasicTypes/Player";
    }
    if (componentTypeName == "State") {
        return "BasicTypes/State";
    }
    if (componentTypeName == "AttackType") {
        return "BasicTypes/AttackType";
    }
    if (componentTypeName == "AbilityType") {
        return "BasicTypes/AbilityType";
    }
    if (componentTypeName == "AttributeType") {
        return "BasicTypes/AttributeType";
    }
    if (componentTypeName == "ValueType") {
        return "BasicTypes/ValueType";
    }
    if (componentTypeName == "Target") {
        return "Target";
    }
    if (componentTypeName == "Bool") {
        return "BasicTypes/BoolCheckBox";
    }
    if (componentTypeName == "Duration") {
        return "BasicTypes/Duration";
    }
    if (componentTypeName == "Int32") {
        return "BasicTypes/BasicTextInput";
    }

    if (componentTypeName == "CardSpecifierCardType") {
        return "BasicTypes/CardSpecifierCardType";
    }
    if (componentTypeName == "CardSpecifierOwner") {
        return "BasicTypes/CardSpecifierOwner";
    }
    if (componentTypeName == "CardSpecifierState") {
        return "BasicTypes/CardSpecifierState";
    }
    if (componentTypeName == "CardSpecifierTrait" ||
            componentTypeName == "CardSpecifierExactName" ||
            componentTypeName == "CardSpecifierNameContains") {
        return "BasicTypes/CardSpecifierTextInput";
    }
    if (componentTypeName == "CardSpecifierLevel" ||
            componentTypeName == "CardSpecifierCost" ||
            componentTypeName == "CardSpecifierPower") {
        return "BasicTypes/CardSpecifierNumber";
    }
    throw std::runtime_error("unimplemented componentTypeName");
}

/*void connectComponent(QString componentName, QQuickItem *qmlObject, QObject* object) {
    if (componentName == "Target") {
        QObject::connect(qmlObject, SIGNAL(componentChanged(QString), object, SLOT(targetChanged())));
    }
}*/
