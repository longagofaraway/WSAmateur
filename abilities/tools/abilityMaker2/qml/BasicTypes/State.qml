import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: stateQml

    property string displayName: 'State'
    signal valueChanged(string newValue, string compId)


    Connections {
        target: parentComponent

        function onSetState(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: stateQml.displayName
        anchors.bottom: stateQml.top
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
