import QtQuick 2.0

Item {
    Rectangle {
        id: menuOpener

        property real cellWidth: menuOpener.width * 0.5
        property real cellHeight: menuOpener.height * 0.08

        width: gGame.width * 0.03
        height: gGame.width * 0.03
        color: "#A0000000"

        states: State {
            name: "opened"
            PropertyChanges {
                target: menuOpener
                x: sideMenu.width
            }
        }

        Column {
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.verticalCenter
            }
            spacing: 4

            Rectangle {
                width: menuOpener.cellWidth
                height: menuOpener.cellHeight
                color: "#90FFFFFF"
            }
            Rectangle {
                width: menuOpener.cellWidth
                height: menuOpener.cellHeight
                color: "#90FFFFFF"
            }
            Rectangle {
                width: menuOpener.cellWidth
                height: menuOpener.cellHeight
                color: "#90FFFFFF"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                menuOpener.state = "opened";
                screenFiller.enabled = true;
                screenFiller.hoverEnabled = true;
            }
        }

        Behavior on x { NumberAnimation { duration: 100 } }
    }

    Rectangle {
        id: sideMenu

        anchors {
            right: menuOpener.left
            top: menuOpener.top
            bottom: parent.bottom
        }
        width: parent.width * 0.09

        color: "#80000000"

        SideMenuItem {
            id: topMenu

            active: false
            text: "Lobby"
            anchors { left: parent.left; right: parent.right }
            y: parent.height * 0.1
            height: parent.height * 0.15
            onMenuClicked: gGame.quitGame()
        }

        SideMenuItem {
            property bool fullscreen: true
            active: false
            text: "Toggle fullscreen"
            fontSize: 14
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: parent.height * 0.15
            onMenuClicked: {
                fullscreen = !fullscreen;
                if (fullscreen)
                    mainWindow.visibility = "FullScreen";
                else
                    mainWindow.visibility = "Windowed";
            }
        }
        SideMenuItem {
            id: exitMenu

            active: false
            text: "Exit"
            anchors {
                left: parent.left
                right: parent.right
                top: topMenu.bottom
            }
            height: parent.height * 0.15
            onMenuClicked: Qt.quit()
        }
    }

    MouseArea {
        id: screenFiller

        anchors {
            left: menuOpener.left
            top: menuOpener.top
            bottom: parent.bottom
            right: parent.right
        }

        hoverEnabled: false
        enabled: false
        onClicked: {
            menuOpener.state = "";
            screenFiller.enabled = false;
            screenFiller.hoverEnabled = false;
        }
    }
}
