import QtQuick 2.0

Rectangle {
    id: sideMenu

    color: "#50000000"

    SideMenuItem {
        id: topMenu

        active: true
        text: "Lobby"
        anchors { left: parent.left; right: parent.right }
        y: parent.height * 0.1
        height: parent.height * 0.15
    }

    SideMenuItem {
        active: false
        text: "Decks"
        anchors {
            left: parent.left;
            right: parent.right;
            top: topMenu.bottom
        }
        height: parent.height * 0.15
    }
}
