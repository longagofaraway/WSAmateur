import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: conditionImpl

    signal editConditions()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: conditionImpl.horizontalCenter
        text: "And"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: conditionImpl.horizontalCenter

        Column {
            Text { text: "Condition" }
            Button {
                text: "Add conditions"
                onClicked: {
                    editConditions();
                }
            }
        }
    }
}
