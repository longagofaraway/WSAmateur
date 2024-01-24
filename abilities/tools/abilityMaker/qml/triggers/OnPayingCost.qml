import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: triggerImpl

    signal abilityTypeChanged(int value)
    signal editTarget()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "On Paying Cost"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        Column {
            Text { text: "Ability type" }
            AbilityType {
                id: abilityType
                Component.onCompleted: {
                    valueChanged.connect(abilityTypeChanged)
                }
            }
        }

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
    }

    function setAbilityType(value) {
        abilityType.setValue(value);
    }
}
