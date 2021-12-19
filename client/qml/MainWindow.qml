import QtQuick 2.12
import QtQuick.Controls 1.1
import QtGraphicalEffects 1.12

import "menu"

Item {
    id: startWindow

    property var imageLinksFileDialog
    property var usernameChoiceDialog

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
            running: true
            to: "#90000000"
            duration: 1000
        }
    }

    Rectangle {
        id: loading
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

    function chooseImageLinksFile() {
        let comp = Qt.createComponent("menu/ChooseFileWindow.qml");
        imageLinksFileDialog = comp.createObject(startWindow);
        imageLinksFileDialog.anchors.centerIn = startWindow;
        loading.visible = false;
        imageLinksFileDialog.fileChosen.connect(startWindow.imageLinksFileChosen);
    }

    function imageLinksFileChosen(path) {
        wsApp.imageLinksFileChosen(path);
    }

    function parseError() {
        imageLinksFileDialog.parseError();
    }

    function parseSuccess() {
        imageLinksFileDialog.destroy();
        loading.visible = true;
    }

    function enterUsername() {
        let comp = Qt.createComponent("menu/ChooseUsername.qml");
        usernameChoiceDialog = comp.createObject(startWindow);
        usernameChoiceDialog.anchors.centerIn = startWindow;
        loading.visible = false;
        usernameChoiceDialog.usernameChosen.connect(startWindow.usernameChosen);
    }
    function usernameChosen(path) {
        wsApp.setUsername(path);
    }
    function usernameSet() {
        usernameChoiceDialog.destroy();
        loading.visible = true;
    }
}
