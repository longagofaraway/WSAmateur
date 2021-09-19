import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal numberChanged(string value)
    signal editAbilities()
    signal durationChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Ability Gain"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

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
            Text { text: "Number of abilities to get" }
            BasicTextInput {
                id: number
                Component.onCompleted: {
                    valueChanged.connect(effectImpl.numberChanged);
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

        Column {
            Text { text: "Duration" }
            ComboBox {
                id: duration
                model: ["0", "1", "2"]
                onCurrentIndexChanged: durationChanged(currentIndex);
            }
        }
    }

    function setNumber(value) {
        number.setValue(value);
    }

    function setDuration(value) {
        duration.currentIndex = value;
    }
}
