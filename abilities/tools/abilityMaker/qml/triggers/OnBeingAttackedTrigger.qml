import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: triggerImpl

    signal editTarget()
    signal attackTypeChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "On Being Attacked"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter
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
            Text { text: "Attack type" }
            AttackType {
                id: attackType
                Component.onCompleted: {
                    valueChanged.connect(attackTypeChanged)
                }
            }
        }
    }

    function setAttackType(value) {
        attackType.setValue(value);
    }
}
