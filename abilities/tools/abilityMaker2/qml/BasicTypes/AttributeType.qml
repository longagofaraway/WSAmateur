import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: attributeType

    property string displayName: 'AttributeType'
    signal valueChanged(string newValue, string compId)

    Connections {
        target: parentComponent

        function onSetAttributeType(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: attributeType.displayName
        anchors.bottom: attributeType.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Power"; value: "Power" }
        ListElement { key: "Soul"; value: "Soul" }
        ListElement { key: "Level"; value: "Level" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
