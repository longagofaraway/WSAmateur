import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: conditionImpl

    signal numModifierChanged(int value)
    signal numValueChanged(string value)
    signal editCard()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: conditionImpl.horizontalCenter
        text: "Condition Check Opened Cards"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: conditionImpl.horizontalCenter

        Column {
            Text { text: "Number" }
            Number {
                id: number

                Component.onCompleted: {
                    number.numModifierChanged.connect(conditionImpl.numModifierChanged);
                    number.valueChanged.connect(conditionImpl.numValueChanged);
                }
            }
        }

        Column {
            id: card
            Text { text: "Card filter" }
            Button {
                text: "Open editor"
                onClicked: {
                    editCard();
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
