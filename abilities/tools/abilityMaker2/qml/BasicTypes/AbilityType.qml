import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    signal valueChanged(string newValue, string compId)

    function setValue(newValue) {
        currentIndex = indexOfValue(newValue);
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Cont"; value: "Cont" }
        ListElement { key: "Auto"; value: "Auto" }
        ListElement { key: "Act"; value: "Act" }
        ListElement { key: "Event"; value: "Event" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
