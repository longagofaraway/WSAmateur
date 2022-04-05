import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal attrChanged(int index)
    signal valueInputChanged(string value)
    signal durationChanged(int value)
    signal gainTypeChanged(int value)
    signal editMultiplier()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Attribute Gain"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text {
                text: "Target"
            }
            Button {
                text: "Open editor"
                onClicked: {
                    editTarget();
                }
            }
        }

        Column {
            Text { text: "Attribute" }
            AttributeType {
                id: attrType
                Component.onCompleted: {
                    valueChanged.connect(attrChanged);
                }
            }
        }

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
            Text { text: "Duration" }
            ComboBox {
                id: duration
                model: ["0", "1", "2"]
                onCurrentIndexChanged: durationChanged(currentIndex);
            }
        }

        Column {
            Text { text: "Gain Type" }
            ValueType {
                id: valueType
                onValueChanged:{
                    if (value == 1)
                        multiplier.enabled = false;
                    else
                        multiplier.enabled = true;
                    gainTypeChanged(value);
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

    function setAttrType(value) {
        attrType.setValue(value);
    }

    function setValueInput(value) {
        valueInput.setValue(value);
    }

    function setDuration(value) {
        duration.currentIndex = value;
    }

    function setGainType(value) {
        valueType.setValue(value);
    }
}
