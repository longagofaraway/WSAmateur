import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: conditionImpl

    signal ownerChanged(int value)
    signal fromChanged(int value)
    signal toChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: conditionImpl.horizontalCenter
        text: "Condition Card Moved"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: conditionImpl.horizontalCenter

        Column {
            Text { text: "Player" }
            Player {
                id: player
                Component.onCompleted: {
                    valueChanged.connect(conditionImpl.ownerChanged);
                }
            }
        }

        Column {
            Text { text: "From" }
            Zone {
                id: from
                Component.onCompleted: {
                    valueChanged.connect(conditionImpl.fromChanged);
                }
            }
        }

        Column {
            Text { text: "To" }
            Zone {
                id: to
                Component.onCompleted: {
                    valueChanged.connect(conditionImpl.toChanged);
                }
            }
        }
    }

    function setOwner(value) {
        player.setValue(value);
    }

    function setFrom(value) {
        from.setValue(value);
    }

    function setTo(value) {
        to.setValue(value);
    }
}
