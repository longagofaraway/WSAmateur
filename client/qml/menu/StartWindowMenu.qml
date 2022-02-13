import QtQuick 2.0

Rectangle {
    id: sideMenu

    property int activeSlot: 1

    color: "#50000000"

    SideMenuItem {
        active: false
        text: "Exit"
        anchors {
            left: parent.left
            right: parent.right
        }
        height: parent.height * 0.15
        onMenuClicked: Qt.quit()
    }
}
