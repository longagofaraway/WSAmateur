import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    property int position
    signal numModifierChangedEx(int pos, int index)
    signal valueChangedEx(int pos, string value)
    signal editMultiplier(int pos)

    spacing: 10

    Number {
        id: number
        Component.onCompleted: {
            numModifierChanged.connect(numModifierChangedProxy);
            valueChanged.connect(valueChangedProxy);
        }
    }

    Button {
        text: "Edit multiplier"
        onClicked: {
            editMultiplier(position);
        }
    }

    function numModifierChangedProxy(index) {
        numModifierChangedEx(position, index);
    }

    function valueChangedProxy(value) {
        valueChangedEx(position, value);
    }

    function setValue(value) {
        number.incomingValue(value);
    }

    function setNumMod(index) {
        number.incomingNumMod(index);
    }
}
