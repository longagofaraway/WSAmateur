import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal triggerIconChanged(int value)
    signal durationChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Trigger Icon Gain"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Target" }
            Button {
                text: "Open editor"
                onClicked: {
                    editTarget();
                }
            }
        }

        Column {
            Text { text: "Trigger Icon" }
            TriggerIcon {
                id: triggerIcon
                onValueChanged: triggerIconChanged(value)
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

    function setTriggerIcon(value) {
        triggerIcon.setValue(value);
    }

    function setDuration(value) {
        duration.currentIndex = value;
    }
}
