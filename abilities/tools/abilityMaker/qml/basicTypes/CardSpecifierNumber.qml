import QtQuick 2.0

Number {
    property int position
    signal numModifierChangedEx(int pos, int index)
    signal valueChangedEx(int pos, string value)

    Component.onCompleted: {
        numModifierChanged.connect(numModifierChangedProxy);
        valueChanged.connect(valueChangedProxy);

    }

    function numModifierChangedProxy(index) {
        numModifierChangedEx(position, index);
    }

    function valueChangedProxy(value) {
        valueChangedEx(position, value);
    }

    function setValue(value) {
        incomingValue(value);
    }

    function setNumMod(index) {
        incomingNumMod(index);
    }
}
