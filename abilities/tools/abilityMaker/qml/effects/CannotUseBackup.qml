import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal backupOrEventChanged(int value)
    signal playerChanged(int value)
    signal durationChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Cannot use backup or event"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Backup or Event" }
            ComboBox {
                id: backupOrEvent
                model: ["Backup", "Event", "Both"]
                onCurrentIndexChanged: {
                    backupOrEventChanged(currentIndex + 1);
                }
            }
        }

        Column {
            Text { text: "Player" }
            Player {
                id: player
                Component.onCompleted: {
                    valueChanged.connect(playerChanged)
                }
            }
        }

        Column {
            Text { text: "Duration" }
            ComboBox {
                id: duration
                model: ["0", "1", "2"]
                onCurrentIndexChanged: durationChanged(currentIndex);
            }
        }
    }

    function setBackupOrEvent(value) {
        backupOrEvent.currentIndex = value - 1;
    }

    function setPlayer(value) {
        player.setValue(value);
    }

    function setDuration(value) {
        duration.currentIndex = value;
    }
}
