import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: trigger

    signal presetChanged(int index)
    signal triggerTypeChanged(int index)
    signal componentReady()
    signal cancel()

    // incoming signals
    signal incomingTriggerType(int index)
    onIncomingTriggerType: {
        triggerTypeCombo.currentIndex = index - 1;
    }

    property real triggerImplY: triggerTypeCombo.y + triggerTypeCombo.height + 10

    x: 0
    y: 0
    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    // Rectangle is transparent for mouse events, so we need this
    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: presetLabel
        anchors.horizontalCenter: trigger.horizontalCenter
        text: "Choose from a preset:"
    }

    ComboBox {
        id: presetCombo
        anchors.top: presetLabel.bottom
        anchors.horizontalCenter: trigger.horizontalCenter
        width: 200
        model: ["Placed from hand to stage", "When this card attacks", "When this becomes reversed",
                "Climax is placed"]
        currentIndex: -1
        onCurrentIndexChanged: presetChanged(currentIndex)
    }

    Text {
        id: delimiterLabel
        anchors { top: presetCombo.bottom; topMargin: 10 }
        anchors.horizontalCenter: trigger.horizontalCenter
        text: "Or specify each field"
        font.pointSize: 12
    }

    Text {
        id: typeLabel
        anchors.right: triggerTypeCombo.left
        anchors.verticalCenter: triggerTypeCombo.verticalCenter
        text: "Trigger type:"
    }

    ComboBox {
        id: triggerTypeCombo
        anchors { top: delimiterLabel.bottom; topMargin: 10 }
        anchors.horizontalCenter: trigger.horizontalCenter
        width: 200
        model: ["Zone change", "On play", "On state change", "On attack", "On backup of this",
                "On trigger reveal", "Phase event", "End of this card's attack",
                "On standby trigger effect", "On being attacked", "On damage cancel",
                "On damage taken cancel", "On paying cost", "When 【ACT】 abillity used"]
        currentIndex: -1
        onCurrentIndexChanged: {
            triggerTypeChanged(currentIndex + 1);
        }
    }

    ConfirmButton {
        onClicked: {
            if (triggerTypeCombo.currentIndex == -1)
                return;
            componentReady()
        }
    }

    CancelButton {
        onClicked: cancel()
    }
}
