import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: player
    property string displayName: 'Player'
    signal valueChanged(string newValue, string compId)

    function setValue(newValue) {
        currentIndex = indexOfValue(newValue);
    }

    Text {
        text: player.displayName
        anchors.bottom: player.top
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
