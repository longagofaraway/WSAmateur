import QtQuick 2.0
import QtGraphicalEffects 1.12

Item {
    id: btn
    width: 190//547
    height: 75//216

    signal clicked()
    property string mText

    Image {
        id: img
        anchors.fill: parent
        source: "qrc:///resources/images/btn_state"
    }

    MouseArea {
        anchors.fill: parent
        onPressed: img.source = "qrc:///resources/images/btn_pressed"
        onReleased: img.source = "qrc:///resources/images/btn_state"
        onClicked: btn.clicked()
    }

    Glow {
        anchors.fill: img
        color: "white"
        source: img
        spread: 0.3
    }

    Text {
        id: buttonText
        anchors { top: parent.top; topMargin: 10 }
        width: parent.width
        text: mText
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        font.pointSize: 20
    }

    function setText(text) {
        buttonText.text = text;
    }
}
