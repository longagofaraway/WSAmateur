import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: conditionImpl

    signal sumChanged(string value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: conditionImpl.horizontalCenter
        text: "Condition Sum Of Levels"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: conditionImpl.horizontalCenter

        Column {
            Text { text: "Sum" }
            BasicTextInput {
                id: valueInput
                Component.onCompleted: {
                    valueChanged.connect(sumChanged);
                }
            }
        }
    }

    function setSum(value) {
        valueInput.setValue(value);
    }
}
