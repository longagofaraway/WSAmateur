import QtQuick 2.0

Player {
    property int position
    signal valueChangedEx(int pos, int value)

    Component.onCompleted: {
        valueChanged.connect(valueChangedProxy);
    }

    function valueChangedProxy(value) {
        valueChangedEx(position, value);
    }

    function setValueEx(value) {
        currentIndex = value - 1;
    }
}
