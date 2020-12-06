import QtQuick 2.0
import QtGraphicalEffects 1.12

Item {

    width: 190//547
    height: 75//216

    Image {
        id: img
        anchors.fill: parent
        source: "qrc:///resources/images/btn_state"
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            img.source = "qrc:///resources/images/btn_pressed"
        }
        onReleased: {
            img.source = "qrc:///resources/images/btn_state"
        }
    }

    Glow {
        anchors.fill: img
        color: "white"
        source: img
        spread: 0.3
    }

    Text {
        anchors.fill: parent
        text: "Start"
        horizontalAlignment: Text.AlignJustify
    }
}
