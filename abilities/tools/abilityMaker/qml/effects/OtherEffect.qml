import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal cardCodeChanged(string value)
    signal effectIdChanged(string value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Hardcoded effect"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Card code" }
            BasicTextInput {
                id: valueInput
                Component.onCompleted: {
                    valueChanged.connect(cardCodeChanged);
                }
            }
        }

        Column {
            Text { text: "Effect id" }
            BasicTextInput {
                id: effectId
                Component.onCompleted: {
                    valueChanged.connect(effectIdChanged);
                }
            }
        }
    }

    function setEffectId(value) {
        effectId.setValue(value);
    }

    function setCardCode(value) {
        valueInput.setValue(value);
    }
}
