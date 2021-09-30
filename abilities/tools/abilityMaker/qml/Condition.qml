import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: condition

    signal conditionTypeChanged(int index)
    signal componentReady()
    signal cancel()

    // incoming signals
    signal incomingConditionType(int index)
    onIncomingConditionType: {
        conditionTypeCombo.currentIndex = index;
    }

    property real conditionImplY: conditionTypeCombo.y + conditionTypeCombo.height + 10

    x: 0
    y: 0
    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    // Rectangle is transparent for mouse events, so we need this
    MouseArea {
        anchors.fill: parent
    }

    Column {
        anchors.horizontalCenter: condition.horizontalCenter
        spacing: 10

    Text {
        id: typeLabel
        text: "Condition type:"
    }

    ComboBox {
        id: conditionTypeCombo
        model: ["No condition", "Is card", "Have cards", "And", "Or", "In battle with this",
                "Sum of levels", "Card's location", "During turn", "Check opened cards",
                "Revealed card", "Player's level"]
        currentIndex: -1
        onCurrentIndexChanged: {
            conditionTypeChanged(currentIndex);
        }
    }
    }

    ConfirmButton {
        onClicked: componentReady()
    }

    CancelButton {
        onClicked: cancel()
    }
}
