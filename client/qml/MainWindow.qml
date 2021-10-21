import QtQuick 2.12
import QtQuick.Controls 1.1
import QtGraphicalEffects 1.12

Item {
    Image {
        id: backgroundImg
        anchors.fill: parent
        source: "qrc:///resources/background_menu"
        fillMode: Image.PreserveAspectCrop
    }

    ColorOverlay {
        id: colorOverlay
        anchors.fill: backgroundImg
        source: backgroundImg
        color: "#00000000"

        ColorAnimation on color {
            running: false
            to: "#B0000000"
            duration: 1000
        }
    }

    Button {
        text: "start"
        onClicked: {
            mainLoader.source = "GameWindow.qml";
        }
    }
}
