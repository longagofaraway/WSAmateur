import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: triggerIcon

    property string displayName: 'Trigger Icon'
    signal valueChanged(string newValue, string compId)

    Connections {
        target: parentComponent

        function onSetTriggerIcon(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: triggerIcon.displayName
        anchors.bottom: triggerIcon.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Soul"; value: "Soul" }
        ListElement { key: "Wind"; value: "Wind" }
        ListElement { key: "Bag"; value: "Bag" }
        ListElement { key: "Door"; value: "Door" }
        ListElement { key: "Book"; value: "Book" }
        ListElement { key: "Shot"; value: "Shot" }
        ListElement { key: "Treasure"; value: "Treasure" }
        ListElement { key: "Gate"; value: "Gate" }
        ListElement { key: "Standby"; value: "Standby" }
        ListElement { key: "Choice"; value: "Choice" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
