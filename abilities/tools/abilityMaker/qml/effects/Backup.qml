import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal powerChanged(string value)
    signal levelChanged(string value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Backup"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Power" }
            BasicTextInput {
                id: powerInput
                Component.onCompleted: {
                    valueChanged.connect(powerChanged);
                }
            }
        }

        Column {
            Text { text: "Level" }
            BasicTextInput {
                id: levelInput
                Component.onCompleted: {
                    valueChanged.connect(levelChanged);
                }
            }
        }
    }

    function setPower(value) {
        powerInput.setValue(value);
    }

    function setLevel(value) {
        levelInput.setValue(value);
    }
}
