import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: duration

    property string displayName: 'Duration'
    signal valueChanged(int newValue, string compId)

    Connections {
        target: parentComponent

        function onSetDuration(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = newValue;
        }
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
    onCurrentValueChanged: valueChanged(currentIndex, componentId);
}
