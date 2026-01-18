import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: itemRoot
    property string componentName
    signal numModifierChanged(string value)
    signal valueChanged(string value)
    signal editMultiplier()

    function setNumMod(newValue) {
        number.setNumMod(newValue);
    }
    function setValue(newValue) {
        number.setValue(newValue);
    }

    Column {
        id: column
        scale: 0.8
        transformOrigin: Item.TopLeft
        spacing: 5

        Text {
            text: itemRoot.componentName
            font.pointSize: 10
            anchors.horizontalCenter: column.horizontalCenter
        }

        Number {
            id: number
            onNumModifierChanged: itemRoot.numModifierChanged(value)
            onValueChanged: itemRoot.valueChanged(value)
        }

        Button {
            id: multiplier
            text: "Multiplier"
            onPressed: itemRoot.editMultiplier()
        }
    }
}
