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
        ListElement { key: "Frontal"; value: "Frontal" }
        ListElement { key: "Side"; value: "Side" }
        ListElement { key: "Direct"; value: "Direct" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
