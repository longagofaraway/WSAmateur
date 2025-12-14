import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: valueType

    property string displayName: 'ValueType'
    signal valueChanged(string newValue, string compId)

    Connections {
        target: parentComponent

        function onSetValueType(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: valueType.displayName
        anchors.bottom: valueType.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Raw Value"; value: "RawValue" }
        ListElement { key: "Multiplier"; value: "Multiplier" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
