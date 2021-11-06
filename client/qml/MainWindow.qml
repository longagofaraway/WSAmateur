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

    Rectangle {
        width: parent.width / 5
        height: parent.height / 6
        anchors.centerIn: parent
        radius: 5
        color: "#B0000000"

        Text {
            anchors.centerIn: parent

            text: "Loading"
            font.pointSize: 30
            color: "white"
        }
    }

    Button {
        text: "start"
        onClicked: {
            mainLoader.source = "GameWindow.qml";
        }
    }
}
