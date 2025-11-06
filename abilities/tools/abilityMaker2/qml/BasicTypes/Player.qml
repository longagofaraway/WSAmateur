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
        ListElement { key: "Player"; value: "Player" }
        ListElement { key: "Opponent"; value: "Opponent" }
        ListElement { key: "Both"; value: "Both" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
