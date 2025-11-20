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
        ListElement { key: "Continuous"; value: "Continuous" }
        ListElement { key: "1"; value: "One" }
        ListElement { key: "2"; value: "Two" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
