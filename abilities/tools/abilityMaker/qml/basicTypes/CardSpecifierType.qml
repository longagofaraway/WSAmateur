import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    property int position
    signal valueChanged(int pos, int value)

    model: ["Card type", "Owner", "Trait", "Exact name", "Name contains",
            "Level", "Level > player's level", "Color", "Cost",
            "Trigger icon", "Has marker", "Power", "Standby target"]
    onCurrentIndexChanged: valueChanged(position, currentIndex + 1)
    currentIndex: -1

    function setType(type) {
        currentIndex = type - 1;
    }
}
