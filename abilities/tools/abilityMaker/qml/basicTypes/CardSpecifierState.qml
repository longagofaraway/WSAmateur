import QtQuick 2.0

import '.'

State {
    id: stateSpecifier

    property int position
    signal valueChangedEx(int pos, int value)

    Component.onCompleted: {
        stateSpecifier.valueChanged.connect(valueChangedProxy);
    }

    function valueChangedProxy(value) {
        valueChangedEx(position, value);
    }

    function setValueEx(value) {
        currentIndex = value - 1;
    }
}
