import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: triggerImpl

    signal editTarget()
    signal cancelledChanged(bool value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "On Damage Cancel"
        font.pointSize: 12
    }

    Row {
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter

        Column {
            id: targetColumn
            Text { text: "Target" }
            Button {
                text: "Open editor"
                onClicked: {
                    editTarget();
                }
            }
        }

        CheckBox {
            id: damageCancelled
            text: "Cancelled"
            checked: false
            onCheckedChanged: {
                cancelledChanged(checked);
            }
        }
    }

    function hideTarget() {
        targetColumn.visible = false;
    }

    function setCancelled(value) {
        damageCancelled.checked = value;
    }
}
