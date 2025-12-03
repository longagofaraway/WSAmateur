import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: attackType
    property string displayName: 'AttackType'

    signal valueChanged(string newValue, string compId)

    function setValue(newValue) {
        currentIndex = indexOfValue(newValue);
    }

    Text {
        text: attackType.displayName
        anchors.bottom: attackType.top
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
