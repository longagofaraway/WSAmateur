import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: multiplierImpl

    signal editTarget()
    signal triggerIconChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: multiplierImpl.horizontalCenter
        text: "Add Trigger Icon Count"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: multiplierImpl.horizontalCenter

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
            Text { text: "Trigger icon" }
            TriggerIcon {
                id: triggerIcon
                onValueChanged: {
                    triggerIconChanged(value);
                }
            }
        }
    }

    function setTriggerIcon(value) {
        triggerIcon.setValue(value);
    }
}
