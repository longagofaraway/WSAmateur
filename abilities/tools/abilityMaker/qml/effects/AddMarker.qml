import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal editDestination()
    signal faceOrientationChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Add Marker"
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
            Text {
                text: "Destination"
            }
            Button {
                text: "Open editor"
                onClicked: {
                    editDestination();
                }
            }
        }

        Column {
            Text { text: "Face Orientation" }
            FaceOrientation {
                id: orientation
                Component.onCompleted: {
                    valueChanged.connect(faceOrientationChanged)
                }
            }
        }
    }

    function setFaceOrientation(value) {
        orientation.setValue(value);
    }
}
