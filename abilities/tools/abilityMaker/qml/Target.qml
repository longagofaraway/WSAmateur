import QtQuick 2.12
import QtQuick.Controls 2.12

import "basicTypes"

Rectangle {
    id: target

    signal typeChanged(int index)
    signal targetModeChanged(int index)
    signal presetChanged(int index)
    signal editCard()
    signal clearCard()
    signal componentReady()
    signal cancel()

    signal numModifierChanged(int index)
    signal valueChanged(string value)

    signal incomingTargetType(int type)
    onIncomingTargetType: {
        typeCombo.currentIndex = type - 1;
    }
    signal incomingTargetMode(int type)
    onIncomingTargetMode: {
        tagetModeCombo.currentIndex = type;
    }
    signal incomingNumModifier(int type)
    onIncomingNumModifier: {
        number.incomingNumMod(type);
    }
    signal incomingNumValue(string value)
    onIncomingNumValue: {
        number.incomingValue(value);
    }

    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    MouseArea {
        anchors.fill: parent
    }

    Column {
        id: typeColumn
        anchors.horizontalCenter: target.horizontalCenter

        Text {
            text: "Target type:"
        }

        ComboBox {
            id: typeCombo
            model: ["This card", "Chosen cards", "Specific card", "Rest of the cards",
                    "Battle opponent", "Mentioned cards", "Character in battle",
                    "Character opposite this", "Last moved card"]
            currentIndex: -1
            onCurrentIndexChanged: {
                if (currentIndex == 2 || currentIndex == 4) {
                    specRow.enabled = true;
                } else {
                    specRow.enabled = false;
                }

                typeChanged(currentIndex + 1);
            }
        }
    }

    Row {
    id: specRow

    anchors { top: typeColumn.bottom; topMargin: 10 }
    anchors.horizontalCenter: target.horizontalCenter
    spacing: 10
    enabled: false

    Column {
        id: targetMode
        Text {
            text: "Target mode:"
        }

        ComboBox {
            id: tagetModeCombo
            model: ["Not specified", "All", "All other", "In front of this",
                    "Front row", "Back row", "Front row other", "Back row other"]
            onCurrentIndexChanged: targetModeChanged(currentIndex)
        }
    }
    Column {
        Text {
            text: "Number:"
        }
        Number {
            id: number

            Component.onCompleted: {
                number.numModifierChanged.connect(target.numModifierChanged);
                number.valueChanged.connect(target.valueChanged);
            }
        }
    }

    CardColumn {
        id: card
        Component.onCompleted: {
            card.editCard.connect(target.editCard);
            card.clearCard.connect(target.clearCard);
        }
    }
    }

    ConfirmButton {
        onClicked: componentReady()
    }

    CancelButton {
        onClicked: cancel()
    }

    function setActualY() {
        y = -mapToItem(root, 0, 0).y + root.pathHeight;
    }
}
