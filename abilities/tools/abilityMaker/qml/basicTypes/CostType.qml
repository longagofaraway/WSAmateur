import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    property int position
    signal valueChanged(int pos, int value)

    model: ["Stock", "Effect"]
    onCurrentIndexChanged: valueChanged(position, currentIndex + 1)
    currentIndex: -1

    function setType(type) {
        currentIndex = type - 1;
    }
}
