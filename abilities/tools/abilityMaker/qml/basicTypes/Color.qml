import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    property int position
    signal valueChanged(int pos, int value)

    model: ["Yellow", "Green", "Red", "Blue", "Purple"]
    onCurrentIndexChanged: valueChanged(position, currentIndex + 1)

    function setValue(value) {
        currentIndex = value - 1;
    }
}
