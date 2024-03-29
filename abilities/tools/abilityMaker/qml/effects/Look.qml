import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editPlace()
    signal numModifierChanged(int value)
    signal numValueChanged(string value)
    signal valueTypeChanged(int value)
    signal editMultiplier()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Look"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Number" }
            Number {
                id: number

                Component.onCompleted: {
                    number.numModifierChanged.connect(effectImpl.numModifierChanged);
                    number.valueChanged.connect(effectImpl.numValueChanged);
                }
            }
        }

        Column {
            id: place
            Text { text: "Place" }
            Button {
                text: "Open editor"
                onClicked: {
                    editPlace();
                }
            }
        }

        Column {
            Text { text: "Value Type" }
            ValueType {
                id: valueType
                onValueChanged:{
                    if (value == 1)
                        multiplier.enabled = false;
                    else
                        multiplier.enabled = true;
                    valueTypeChanged(value);
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

    function setNumModifier(value) {
        number.incomingNumMod(value);
    }

    function setNumValue(value) {
        number.incomingValue(value);
    }

    function setValueType(value) {
        valueType.setValue(value);
    }
}
