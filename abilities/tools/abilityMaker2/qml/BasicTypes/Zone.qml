import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: zone
    signal valueChanged(string newValue, string compId)

    function setValue(newValue) {
        currentIndex = indexOfValue(newValue);
    }

    Text {
        text: "Zone"
        anchors.bottom: zone.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Stage"; value: "Stage" }
        ListElement { key: "Waiting Room"; value: "WaitingRoom" }
        ListElement { key: "Deck"; value: "Deck" }
        ListElement { key: "Clock"; value: "Clock" }
        ListElement { key: "Hand"; value: "Hand" }
        ListElement { key: "Memory"; value: "Memory" }
        ListElement { key: "Stock"; value: "Stock" }
        ListElement { key: "Level"; value: "Level" }
        ListElement { key: "Climax"; value: "Climax" }
        ListElement { key: "Not Specified"; value: "NotSpecified" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
