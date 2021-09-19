import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    property int costCount: 0
    property int kOffset: 0
    property real btnOffset: costCount * kOffset

    signal cancel()
    signal componentReady()

    signal addCost()

    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    MouseArea {
        anchors.fill: parent
    }

    ConfirmButton {
        onClicked: componentReady()
    }

    CancelButton {
        onClicked: cancel()
    }

    Text {
        id: label
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Cost"
        font.pointSize: 12
    }

    Button {
        x: btnOffset
        y: 100
        width: 100
        height: 100

        Text {
            anchors.centerIn: parent
            text: "+"
            font.pointSize: 24
        }

        onClicked: addCost()
    }

    function setActualY() {
        y = -mapToItem(root, 0, 0).y + root.pathHeight;
    }
}
