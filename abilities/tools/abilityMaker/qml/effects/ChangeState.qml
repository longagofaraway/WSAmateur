import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal cardStateChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Change State"
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
            Text { text: "State" }
            State {
                id: cardState
                Component.onCompleted: {
                    valueChanged.connect(cardStateChanged)
                }
            }
        }
    }

    function setCardState(value) {
        cardState.setValue(value);
    }
}
