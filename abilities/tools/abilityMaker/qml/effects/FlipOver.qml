import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal numModifierChanged(int value)
    signal numValueChanged(string value)
    signal editCard()
    signal editEffects()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Flip Over"
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
            id: card
            Text { text: "For each" }
            Button {
                text: "Open editor"
                onClicked: {
                    editCard();
                }
            }
        }

        Column {
            Text { text: "Effects" }
            Button {
                text: "Add effects"
                onClicked: {
                    editEffects();
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
}
