import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: triggerImpl

    signal editTarget()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "On Play Trigger"
        font.pointSize: 12
    }

    Column {
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter
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
