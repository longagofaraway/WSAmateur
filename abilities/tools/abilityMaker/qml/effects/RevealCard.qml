import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editCard()
    signal revealTypeChanged(int value)
    signal numModifierChanged(int value)
    signal numValueChanged(string value)

    signal incomingRevealType(int value)
    onIncomingRevealType: {
        revealType.currentIndex = value - 1;
    }

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Reveal Card"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Reveal type" }
            ComboBox {
                id: revealType
                model: ["Top deck", "Chosen cards", "From hand"]
                onCurrentIndexChanged: {
                    if (currentIndex == 2)
                        card.enabled = true;
                    else
                        card.enabled = false;
                    revealTypeChanged(currentIndex + 1);
                }
            }
        }

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
            enabled: false
            Text { text: "Card filter" }
            Button {
                text: "Open editor"
                onClicked: {
                    editCard();
                }
            }
        }
    }

    function setRevealType(value) {
        revealType.currentIndex = value - 1;
    }

    function setNumModifier(value) {
        number.incomingNumMod(value);
    }

    function setNumValue(value) {
        number.incomingValue(value);
    }
}
