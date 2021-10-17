import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.15

Window {
    id: mainWindow
    visible: true
    visibility: "FullScreen"
    width: 640
    height: 480
    title: qsTr("Hello World")

    Loader {
        id: mainLoader
        anchors.fill: parent
        source: "GameWindow.qml"
    }
}
