import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    signal valueChanged(int value)

    signal setValue(int value)
    onSetValue: {
        currentIndex = value - 1;
    }

    model: ["Look/Reveal", "Specific Place", "Last Moved Cards", "Marker"]
    onCurrentIndexChanged: valueChanged(currentIndex + 1)
}
