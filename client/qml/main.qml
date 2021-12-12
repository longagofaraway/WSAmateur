import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.15

import wsamateur 1.0

Window {
    id: mainWindow
    visible: true
    visibility: "FullScreen"
    width: 640
    height: 480
    title: qsTr("WSAmateur")

    WSApplication {
        id: wsApp
        onStartGame: mainLoader.source = "GameWindow.qml"
        onLoadLobby: mainLoader.source = "menu/LobbyWindow.qml"

        function switchToDeckMenu() {
            mainLoader.source = "menu/DecksWindow.qml";
        }
        function switchToLobby() {
            mainLoader.source = "menu/LobbyWindow.qml"
        }
    }

    Loader {
        id: mainLoader
        anchors.fill: parent
        source: "MainWindow.qml"
        focus: true
    }
}
