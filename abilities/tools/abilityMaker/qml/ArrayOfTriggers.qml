import QtQuick 2.12
import QtQuick.Controls 2.12

import "basicTypes"

Rectangle {
    id: arrayOfTrigger

    property int specCount: 0
    property int kOffset: 0
    property real btnOffset: specCount * kOffset

    signal addTrigger()
    signal componentReady()
    signal cancel()

    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    MouseArea {
        anchors.fill: parent
    }

    Button {
        x: btnOffset
        y: 100

        text: "Add trigger"
        onClicked: addTrigger()
    }

    ConfirmButton {
        onClicked: componentReady()
    }

    CancelButton {
        onClicked: cancel()
    }

    function setActualY() {
        y = -mapToItem(root, 0, 0).y + root.pathHeight;
    }
}
