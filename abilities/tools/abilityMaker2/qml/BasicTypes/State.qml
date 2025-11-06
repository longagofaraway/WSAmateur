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
        ListElement { key: "Standing"; value: "Standing" }
        ListElement { key: "Rested"; value: "Rested" }
        ListElement { key: "Reversed"; value: "Reversed" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
