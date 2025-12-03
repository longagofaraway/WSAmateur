import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: phase
    property string displayName: 'Phase'

    signal valueChanged(string newValue, string compId)

    function setValue(newValue) {
        currentIndex = indexOfValue(newValue);
    }

    Text {
        text: phase.displayName
        anchors.bottom: phase.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Mulligan"; value: "Mulligan" }
        ListElement { key: "Stand phase"; value: "StandPhase" }
        ListElement { key: "Draw phase"; value: "DrawPhase" }
        ListElement { key: "Clock phase"; value: "ClockPhase" }
        ListElement { key: "Main phase"; value: "MainPhase" }
        ListElement { key: "Climax phase"; value: "ClimaxPhase" }
        ListElement { key: "Attack phase"; value: "AttackPhase" }
        ListElement { key: "End phase"; value: "EndPhase" }
        ListElement { key: "Attack declaration step"; value: "AttackDeclarationStep" }
        ListElement { key: "Trigger step"; value: "TriggerStep" }
        ListElement { key: "Counter step"; value: "CounterStep" }
        ListElement { key: "Damage step"; value: "DamageStep" }
        ListElement { key: "Battle step"; value: "BattleStep" }
        ListElement { key: "Encore step"; value: "EncoreStep" }
        ListElement { key: "Not Specified"; value: "NotSpecified" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
