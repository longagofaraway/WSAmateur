import QtQuick 2.0

Rectangle {
    id: sideMenu

    property int activeSlot: 1

    color: "#50000000"

    SideMenuItem {
        id: topMenu

        active: activeSlot == 1
        text: "Lobby"
        anchors { left: parent.left; right: parent.right }
        y: parent.height * 0.1
        height: parent.height * 0.15
        onMenuClicked: wsApp.switchToLobby()
    }

    SideMenuItem {
        active: activeSlot == 2
        text: "Decks"
        anchors {
            left: parent.left
            right: parent.right
            top: topMenu.bottom
        }
        height: parent.height * 0.15
        onMenuClicked: wsApp.switchToDeckMenu()
    }

    SideMenuItem {
        active: false
        text: "Exit"
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: parent.height * 0.15
        onMenuClicked: Qt.quit()
    }
}
