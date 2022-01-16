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
        onImageLinksFileNotFound: mainLoader.item.chooseImageLinksFile()
        onImageFileParsed: mainLoader.item.parseSuccess()
        onImageFileParseError: mainLoader.item.parseError()
        onUsernameNotFound: mainLoader.item.enterUsername()
        onUsernameSet: mainLoader.item.usernameSet()

        function switchToDeckMenu() {
            mainLoader.source = "menu/DecksWindow.qml";
        }
        function switchToLobby() {
            mainLoader.source = "menu/LobbyWindow.qml";
        }
        function startWindow() {
            mainLoader.source = "StartWindow.qml";
        }
    }

    Loader {
        id: mainLoader
        anchors.fill: parent
        source: "StartWindow.qml"
        focus: true
    }
}
