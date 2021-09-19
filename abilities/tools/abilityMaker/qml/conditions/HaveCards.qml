import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: conditionImpl

    signal ownerChanged(int value)
    signal numModifierChanged(int value)
    signal numValueChanged(string value)
    signal editCard()
    signal editPlace()
    signal excludingThisChanged(bool value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: conditionImpl.horizontalCenter
        text: "Condition Have Cards"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: conditionImpl.horizontalCenter

        Column {
            Text { text: "Who" }
            Player {
                id: player
                Component.onCompleted: {
                    valueChanged.connect(conditionImpl.ownerChanged);
                }
            }
        }

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

        CheckBox {
            id: excludingThis
            text: "Excluding this"
            checked: false
            onCheckedChanged: {
                excludingThisChanged(checked);
            }
        }
    }

    function setNumModifier(value) {
        number.incomingNumMod(value);
    }

    function setNumValue(value) {
        number.incomingValue(value);
    }

    function setOwner(value) {
        player.setValue(value);
    }

    function setExcludingThis(value) {
        excludingThis.checked = value;
    }
}
