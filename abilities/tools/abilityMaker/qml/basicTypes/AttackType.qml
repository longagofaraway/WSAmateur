import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    signal valueChanged(int value)

    signal setValue(int value)
    onSetValue: {
        currentIndex = value;
    }

    model: ["Any", "Front", "Side", "Direct"]
    onCurrentIndexChanged: valueChanged(currentIndex)
}
