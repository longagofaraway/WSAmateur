import QtQuick 2.12
import QtQml.Models 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 1.1

Window {
    visible: true
    visibility: "FullScreen"
    width: 640
    height: 480
    title: qsTr("Hello World")

    Image {
        id: backgroundImg
        anchors.fill: parent
        source: "qrc:///resources/background.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    ColorOverlay {
        anchors.fill: backgroundImg
        source: backgroundImg
        color: "#B0000000"
    }

    Loader {
        id: mainLoader
        anchors.fill: parent
        source: "MainWindow.qml"
    }
}
