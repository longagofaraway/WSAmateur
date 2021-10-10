import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal valueInputChanged(string value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Deal Damage"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Value" }
            BasicTextInput {
                id: valueInput
                Component.onCompleted: {
                    valueChanged.connect(valueInputChanged);
                }
            }
        }
    }

    function setValueInput(value) {
        valueInput.setValue(value);
    }

}
