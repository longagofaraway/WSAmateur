import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal editPlace()
    signal positionChanged(int value)

    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 100

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Choose Card"
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
            id: place
            Text { text: "From" }
            Button {
                text: "Open editor"
                onClicked: {
                    editPlace();
                }
            }
        }

        Column {
            Text { text: "To" }
            ComboBox {
                width: 160
                id: position
                model: ["Not specified", "Top", "Bottom", "Front row", "Back row", "Empty slot front row",
                        "Empty slot back row", "Empty slot", "Slot this was in rested", "Slot this was in",
                        "Slot target was in"]
                onCurrentIndexChanged: positionChanged(currentIndex)
            }
        }
    }

    function setPosition(value) {
        position.currentIndex = value;
    }
}
