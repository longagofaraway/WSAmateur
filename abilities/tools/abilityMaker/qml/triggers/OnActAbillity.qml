import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: triggerImpl

    signal ownerChanged(int index)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "On 【ACT】 abillity"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter

        Column {
            Text { text: "Player" }
            Player {
                id: playerCombo
                Component.onCompleted: {
                    valueChanged.connect(ownerChanged);
                }
            }
        }
    }

    function setPlayer(value) {
        playerCombo.setValue(value);
    }
}
