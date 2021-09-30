import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    signal valueChanged(int value)

    signal setValue(int value)
    onSetValue: {
        currentIndex = value;
    }

    model: ["Not specified", "Stage", "Waiting Room", "Deck", "Clock",
            "Hand", "Memory", "Stock", "Level", "Climax"]
    currentIndex: -1
    onCurrentIndexChanged: valueChanged(currentIndex)
}
