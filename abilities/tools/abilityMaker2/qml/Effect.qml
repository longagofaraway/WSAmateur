import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: effect

    signal effectTypeChanged(string value)

    /*function setValue(newValue) {
        triggerTypeCombo.dontTrigger = true;
        triggerTypeCombo.currentIndex = triggerTypeCombo.indexOfValue(newValue);
    }*/

    Text {
        id: header
        anchors { top: effect.top; topMargin: 10 }
        anchors.horizontalCenter: effect.horizontalCenter
        font.pointSize: 12
        text: "Effect"
    }

}
