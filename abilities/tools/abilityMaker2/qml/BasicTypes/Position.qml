import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: position

    property string displayName: 'Position'
    signal valueChanged(string newValue, string compId)

    Connections {
        target: parentComponent

        function onSetPosition(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: position.displayName
        anchors.bottom: position.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Not Specified"; value: "NotSpecified" }
        ListElement { key: "Top"; value: "Top" }
        ListElement { key: "Bottom"; value: "Bottom" }
        ListElement { key: "Front Row"; value: "FrontRow" }
        ListElement { key: "Back Row"; value: "BackRow" }
        ListElement { key: "Empty Slot Front Row"; value: "EmptySlotFrontRow" }
        ListElement { key: "Empty Slot Back Row"; value: "EmptySlotBackRow" }
        ListElement { key: "Empty Slot"; value: "EmptySlot" }
        ListElement { key: "Slot This Was In Rested"; value: "SlotThisWasInRested" }
        ListElement { key: "Slot This Was In"; value: "SlotThisWasIn" }
        ListElement { key: "Slot Target Was In"; value: "SlotTargetWasIn" }
        ListElement { key: "Opposite Character"; value: "OppositeCharacter" }
        ListElement { key: "Front Row Middle Position"; value: "FrontRowMiddlePosition" }
        ListElement { key: "Empty Front Row Middle Position"; value: "EmptyFrontRowMiddlePosition" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
