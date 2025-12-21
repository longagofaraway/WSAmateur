import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: placeType

    property string displayName: 'PlaceType'
    signal valueChanged(string newValue, string compId)

    Connections {
        target: parentComponent

        function onSetPlaceType(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: placeType.displayName
        anchors.bottom: placeType.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Selection"; value: "Selection" }
        ListElement { key: "Specific Place"; value: "SpecificPlace" }
        ListElement { key: "Last Moved Cards"; value: "LastMovedCards" }
        ListElement { key: "Marker"; value: "Marker" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
