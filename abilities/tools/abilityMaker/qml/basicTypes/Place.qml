import QtQuick 2.12
import QtQuick.Controls 2.12

import ".."

Rectangle {
    signal componentReady()
    signal cancel()

    signal positionChanged(int value)
    signal zoneChanged(int value)
    signal ownerChanged(int value)

    signal incomingPosition(int value)
    signal incomingZone(int value)
    signal incomingOwner(int value)

    onIncomingPosition: {
        position.currentIndex = value;
    }
    onIncomingZone: {
        zone.setValue(value);
    }
    onIncomingOwner: {
        player.setValue(value);
    }

    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    MouseArea {
        anchors.fill: parent
    }

    Row {
        anchors { top: confirmButton.bottom; topMargin: 10 }
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
    Column {
        Text { text: "Position" }
        ComboBox {
            width: 160
            id: position
            model: ["Not specified", "Top", "Bottom", "Front row", "Back row", "Empty slot front row",
                    "Empty slot back row", "Empty slot", "Slot this was in rested", "Slot this was in",
                    "Slot target was in", "Opposite character", "Middle position", "Empty middle position"]
            onCurrentIndexChanged: positionChanged(currentIndex)
        }
    }
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
            id: player
            Component.onCompleted: {
                valueChanged.connect(ownerChanged)
            }
        }
    }
    }

    ConfirmButton {
        id: confirmButton
        onClicked: componentReady()
    }

    CancelButton {
        onClicked: cancel()
    }

    function setActualY() {
        y = -mapToItem(root, 0, 0).y + root.pathHeight;
    }
}
