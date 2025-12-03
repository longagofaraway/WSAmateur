import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: duration
    property string displayName: 'Duration'

    signal valueChanged(string newValue, string compId)

    function setValue(newValue) {
        currentIndex = indexOfValue(newValue);
    }

    Text {
        text: duration.displayName
        anchors.bottom: duration.top
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
