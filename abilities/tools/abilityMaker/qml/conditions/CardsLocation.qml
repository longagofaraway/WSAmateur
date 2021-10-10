import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: conditionImpl

    signal editPlace()
    signal editTarget()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: conditionImpl.horizontalCenter
        text: "Condition Card's Location"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: conditionImpl.horizontalCenter

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
            Text { text: "Place" }
            Button {
                text: "Open editor"
                onClicked: {
                    editPlace();
                }
            }
        }
    }
}
