import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: multiplier

    property string displayName: 'Multiplier'

    signal multiplierTypeChanged(string value)

    function setValue(newValue) {
        multiplierTypeCombo.dontTrigger = true;
        multiplierTypeCombo.currentIndex = multiplierTypeCombo.indexOfValue(newValue);
    }

    width: 250
    height: 100
    border.color: "black"
    border.width: 1
    radius: 5
    visible: false

    Text {
        id: header

        font.pointSize: 12
        text: multiplier.displayName
        anchors { horizontalCenter: multiplier.horizontalCenter; bottom: multiplier.top; bottomMargin: 10 }
    }

    ComboBox {
        id: multiplierTypeCombo

        property bool dontTrigger: false
        anchors { top: multiplier.top; topMargin: 10; horizontalCenter: multiplier.horizontalCenter }
        width: 200
        textRole: "key"
        valueRole: "value"
        model: ListModel {
            ListElement { key: "For Each"; value: "ForEach" }
            ListElement { key: "Times Level"; value: "TimesLevel" }
            ListElement { key: "Add Level"; value: "AddLevel" }
            ListElement { key: "Add Trigger Number"; value: "AddTriggerNumber" }
            ListElement { key: "Previous Damage"; value: "PreviousDamage" }
        }
        currentIndex: -1
        onCurrentValueChanged: {
            if (dontTrigger) {
                dontTrigger = false;
                return;
            }

            multiplierTypeChanged(currentValue);
        }
    }
}
