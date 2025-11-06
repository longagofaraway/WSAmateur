import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: itemRoot
    signal valueChanged(string newValue)

    function setValue(newValue) {
        comboBox.currentIndex = comboBox.indexOfValue(newValue);
    }

    Column {
        id: column
        scale: 0.8
        transformOrigin: Item.TopLeft
        spacing: 5

        Text {
            text: "Owner"
            font.pointSize: 10
            anchors.horizontalCenter: column.horizontalCenter
        }

        Player {
            id: comboBox

            font.pointSize: 10
            onCurrentValueChanged: itemRoot.valueChanged(currentValue);
        }
    }
}
