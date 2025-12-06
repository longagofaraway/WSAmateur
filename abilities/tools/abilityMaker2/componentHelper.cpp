#include "componentHelper.h"

#include <stdexcept>

#include "abilities.h"

QString getBasicComponentQmlPath(QString componentTypeName) {
    QHash<QString, QString> kComponentQmlPath = {
        {"Zone", "BasicTypes/Zone"},
        {"Phase", "BasicTypes/Phase"},
        {"PhaseState", "BasicTypes/PhaseState"},
        {"Player", "BasicTypes/Player"},
        {"State", "BasicTypes/State"},
        {"AttackType", "BasicTypes/AttackType"},
        {"AbilityType", "BasicTypes/AbilityType"},
        {"AttributeType", "BasicTypes/AttributeType"},
        {"ValueType", "BasicTypes/ValueType"},
        {"Target", "Target"},
        {"Card", "Card"},
        {"Bool", "BasicTypes/BoolCheckBox"},
        {"Duration", "BasicTypes/Duration"},
        {"Int32", "BasicTypes/BasicTextInput"},
        {"CardSpecifierCardType", "BasicTypes/CardSpecifierCardType"},
        {"CardSpecifierOwner", "BasicTypes/CardSpecifierOwner"},
        {"CardSpecifierState", "BasicTypes/CardSpecifierState"},
        {"CardSpecifierTrait", "BasicTypes/CardSpecifierTextInput"},
        {"CardSpecifierExactName", "BasicTypes/CardSpecifierTextInput"},
        {"CardSpecifierNameContains", "BasicTypes/CardSpecifierTextInput"},
        {"CardSpecifierLevel", "BasicTypes/CardSpecifierNumber"},
        {"CardSpecifierCost", "BasicTypes/CardSpecifierNumber"},
        {"CardSpecifierPower", "BasicTypes/CardSpecifierNumber"}
    };
    if (!kComponentQmlPath.contains(componentTypeName)) {
        throw std::runtime_error("unimplemented componentTypeName");
    }
    return kComponentQmlPath[componentTypeName];
}

