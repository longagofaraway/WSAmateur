import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editChooseOne()
    signal editChooseTwo()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Swap Cards"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            id: chooseOne
            Text { text: "Choose one" }
            Button {
                text: "Open editor"
                onClicked: {
                    editChooseOne();
                }
            }
        }

        Column {
            id: chooseTwo
            Text { text: "Choose two" }
            Button {
                text: "Open editor"
                onClicked: {
                    editChooseTwo();
                }
            }
        }
    }
}
