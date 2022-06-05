import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    signal valueChanged(int value)

    signal setValue(int value)
    onSetValue: {
        currentIndex = value - 1;
    }

    model: ["Cont", "Auto", "Act"]
    currentIndex: -1
    onCurrentIndexChanged: valueChanged(currentIndex + 1)
}
