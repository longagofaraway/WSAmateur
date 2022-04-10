import QtQuick 2.12
import QtQuick.Controls 2.12

import ".."

Rectangle {
    id: multiplier

    signal multiplierTypeChanged(int index)
    signal componentReady()
    signal cancel()

    // incoming signals
    signal incomingMultiplierType(int index)
    onIncomingMultiplierType: {
        multiplierTypeCombo.currentIndex = index - 1;
    }

    property real multiplierImplY: multiplierTypeCombo.y + multiplierTypeCombo.height + 10

    x: 0
    y: 0
    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    // Rectangle is transparent for mouse events, so we need this
    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: typeLabel
        anchors.right: multiplierTypeCombo.left
        anchors.verticalCenter: multiplierTypeCombo.verticalCenter
        text: "Multiplier type:"
    }

    ComboBox {
        id: multiplierTypeCombo
        width: implicitWidth
        anchors.horizontalCenter: multiplier.horizontalCenter
        model: ["For Each", "Times Level", "Add Level"]
        currentIndex: -1
        onCurrentIndexChanged: {
            multiplierTypeChanged(currentIndex + 1);
        }
    }

    ConfirmButton {
        onClicked: {
            if (multiplierTypeCombo.currentIndex == -1)
                return;
            componentReady()
        }
    }

    CancelButton {
        onClicked: cancel()
    }

    function setActualY() {
        y = -mapToItem(root, 0, 0).y + root.pathHeight;
    }
}
