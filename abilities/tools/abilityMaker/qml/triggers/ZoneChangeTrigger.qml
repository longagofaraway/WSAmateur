import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: triggerImpl

    signal editTarget()
    signal fromChanged(int index)
    signal toChanged(int index)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "Zone Change Trigger"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter

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
            Text { text: "From" }
            Zone {
                id: fromZone
                Component.onCompleted: {
                    valueChanged.connect(fromChanged);
                }
            }
        }
        Column {
            Text { text: "To" }
            Zone {
                id: toZone
                Component.onCompleted: {
                    valueChanged.connect(toChanged);
                }
            }
        }
    }

    function setFrom(value) {
        fromZone.setValue(value);
    }

    function setTo(value) {
        toZone.setValue(value);
    }
}
