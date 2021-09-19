import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    property int position
    signal valueChanged(int pos, int value)

    model: ["Soul", "Wind", "Pool", "Door", "Book", "Shot", "Treasure", "Gate",
            "Standby", "Choice"]
    onCurrentIndexChanged: valueChanged(position, currentIndex + 1)

    function setValue(value) {
        currentIndex = value - 1;
    }
}
