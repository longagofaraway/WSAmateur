import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal effectNumChanged(string value)
    signal effectTimesChanged(string value)
    signal editAbilities()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Perform Effect"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text { text: "Number of effects" }
            BasicTextInput {
                id: effectsNum
                Component.onCompleted: {
                    valueChanged.connect(effectImpl.effectNumChanged);
                }
            }
        }

        Column {
            Text { text: "Number of times" }
            BasicTextInput {
                id: effectTimes
                Component.onCompleted: {
                    valueChanged.connect(effectImpl.effectTimesChanged);
                }
            }
        }

        Column {
            Text {
                text: "Abilities"
            }
            Button {
                text: "Open editor"
                onClicked: {
                    editAbilities();
                }
            }
        }
    }

    function setEffectNum(value) {
        effectsNum.setValue(value);
    }

    function setEffectTimes(value) {
        effectTimes.setValue(value);
    }
}
