import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: order

    property string displayName: 'Order'
    signal valueChanged(string newValue, string compId)

    Connections {
        target: parentComponent

        function onSetOrder(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: order.displayName
        anchors.bottom: order.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Not Specified"; value: "NotSpecified" }
        ListElement { key: "Any"; value: "Any" }
        ListElement { key: "Same"; value: "Same" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
