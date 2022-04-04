import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal zoneChanged(int value)
    signal playerChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Shuffle"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Zone" }
            Zone {
                id: zone
                Component.onCompleted: {
                    valueChanged.connect(zoneChanged)
                }
            }
        }

        Column {
            Text { text: "Owner" }
            Player {
                id: owner
                Component.onCompleted: {
                    valueChanged.connect(playerChanged)
                }
            }
        }
    }

    function setZone(value) {
        zone.setValue(value);
    }

    function setPlayer(value) {
        owner.setValue(value);
    }
}
