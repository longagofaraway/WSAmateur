import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    signal valueChanged(int value)

    signal setValue(int value)
    onSetValue: {
        currentIndex = value - 1;
    }

    model: ["Mulligan", "Stand phase", "Draw phase", "Clock phase", "Main phase",
            "Climax phase", "Attack phase", "End phase", "Attack declaration step",
            "Trigger step", "Counter step", "Damage step", "Battle step", "Encore step"]
    onCurrentIndexChanged: valueChanged(currentIndex + 1)
}
