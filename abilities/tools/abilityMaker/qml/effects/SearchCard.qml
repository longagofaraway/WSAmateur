import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editPlace()
    signal editCard()
    signal numModifierChanged(int value)
    signal numValueChanged(string value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Search Card"
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
            id: from
            Text { text: "Place" }
            Button {
                text: "Open editor"
                onClicked: {
                    editPlace();
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
