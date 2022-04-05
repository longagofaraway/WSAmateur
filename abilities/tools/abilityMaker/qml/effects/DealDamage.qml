import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal valueInputChanged(string value)
    signal damageTypeChanged(int value)
    signal editMultiplier()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Deal Damage"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Value" }
            BasicTextInput {
                id: valueInput
                Component.onCompleted: {
                    valueChanged.connect(valueInputChanged);
                }
            }
        }

        Column {
            Text { text: "Damage Type" }
            ValueType {
                id: valueType
                onValueChanged:{
                    if (value == 1)
                        multiplier.enabled = false;
                    else
                        multiplier.enabled = true;
                    damageTypeChanged(value);
                }
            }
        }

        Column {
            id: multiplier
            enabled: false
            Text { text: "Multiplier" }
            Button {
                text: "Open editor"
                onClicked: {
                    editMultiplier();
                }
            }
        }
    }

    function setValueInput(value) {
        valueInput.setValue(value);
    }

    function setDamageType(value) {
        valueType.setValue(value);
    }
}
