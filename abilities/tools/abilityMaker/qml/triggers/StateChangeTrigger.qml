import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: triggerImpl

    signal cardStateChanged(int value)
    signal editTarget()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "On Battle Opponent Reversed"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        Column {
            Text { text: "Target state" }
            State {
                id: cardState
                Component.onCompleted: {
                    valueChanged.connect(cardStateChanged)
                }
            }
        }

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
    }

    function setCardState(value) {
        cardState.setValue(value);
    }
}
