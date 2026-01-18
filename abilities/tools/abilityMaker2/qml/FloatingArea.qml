import QtQuick 2.0
import QtQuick.Controls 2.12

Rectangle {
    width: 100
    height: 100
    radius: 5
    border.width: 2
    border.color: "black"

    signal areaClosing()

    MouseArea {
        anchors.fill: parent
    }

    Button {
        text: "Ok"
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 2 }
        onClicked: areaClosing()
    }
}
