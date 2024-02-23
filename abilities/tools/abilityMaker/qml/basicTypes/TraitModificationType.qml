import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    signal valueChanged(int value)

    signal setValue(int value)
    onSetValue: {
        currentIndex = value - 1;
    }

    model: ["Gain", "Loss"]
    onCurrentIndexChanged: valueChanged(currentIndex + 1)
}
