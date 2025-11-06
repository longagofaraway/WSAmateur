import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: trigger

    signal triggerTypeChanged(string value)

    function setValue(newValue) {
        triggerTypeCombo.dontTrigger = true;
        triggerTypeCombo.currentIndex = triggerTypeCombo.indexOfValue(newValue);
    }

    Text {
        id: header
        anchors { top: trigger.top; topMargin: 10 }
        anchors.horizontalCenter: trigger.horizontalCenter
        font.pointSize: 12
        text: "Trigger"
    }

    ComboBox {
        id: triggerTypeCombo

        property bool dontTrigger: false
        anchors { top: header.bottom; topMargin: 10 }
        anchors.horizontalCenter: trigger.horizontalCenter
        width: 200
        textRole: "key"
        valueRole: "value"
        model: ListModel {
            ListElement { key: "Zone change"; value: "OnZoneChange" }
            ListElement { key: "On play"; value: "OnPlay" }
            ListElement { key: "On state change"; value: "OnStateChange" }
            ListElement { key: "On attack"; value: "OnAttack" }
            ListElement { key: "On backup of this"; value: "OnBackupOfThis" }
            ListElement { key: "On trigger reveal"; value: "OnTriggerReveal" }
            ListElement { key: "Phase event"; value: "OnPhaseEvent" }
            ListElement { key: "End of this card's attack"; value: "OnEndOfThisCardsAttack" }
            ListElement { key: "On standby trigger effect"; value: "OnOppCharPlacedByStandbyTriggerReveal" }
            ListElement { key: "On being attacked"; value: "OnBeingAttacked" }
            ListElement { key: "On damage cancel"; value: "OnDamageCancel" }
            ListElement { key: "On damage taken cancel"; value: "OnDamageTakenCancel" }
            ListElement { key: "On paying cost"; value: "OnPayingCost" }
            ListElement { key: "When 【ACT】 abillity used"; value: "OnActAbillity" }
        }
        currentIndex: -1
        onCurrentValueChanged: {
            if (dontTrigger) {
                dontTrigger = false;
                return;
            }

            triggerTypeChanged(currentValue);
        }
    }
}
