import QtQuick 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.15

import wsamateur 1.0

Lobby {
    id: lobby

    property real tableCellWidth: 500
    property real tableCellHeight: 40
    property int onlineCount: 0

    onLobbyCreated: {
        wsApp.initLobby(lobby);
    }
    onUserCountChanged: {
        onlineCount = userCount;
    }

    Image {
        id: backgroundImg
        anchors.fill: parent
        source: "qrc:///resources/background_menu_side"
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

    SideMenu {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.width * 0.09
    }

    Rectangle {
        id: tableRectangle

        property real margin: 30

        anchors.verticalCenter: parent.verticalCenter
        x: lobby.width / 4
        color: "#50000000"
        width: tableCellWidth * lobby.userListModel.columnCount()
        height: 500 + tableCellHeight + margin * 2

        HorizontalHeaderView {
            id: horizontalHeader
            width: contentWidth
            model: lobby.userListModel
            // https://bugreports.qt.io/browse/QTBUG-88555
            // syncView: tableView
            z:100

            anchors {
                bottom: tableView.top
                left: tableView.left
                right: tableView.right
            }

            delegate: Rectangle {
                implicitWidth: tableCellWidth
                implicitHeight: tableCellHeight
                color: "#D0333333"
                Text {
                    anchors.fill: parent
                    text: display
                    color: "white"
                    font.pointSize: 18
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        TableView {
            id: tableView

            anchors.fill: parent
            clip: true

            ScrollBar.vertical: ScrollBar {}

            model: lobby.userListModel

            delegate: Rectangle {
                id: cell
                implicitWidth: tableCellWidth
                implicitHeight: tableCellHeight
                border.width: 1
                border.color: "#00000000"
                color: model.selected ? "white" : "#00000000"

                Text {
                    anchors.fill: parent
                    text: tableData
                    color: model.selected ? "black" : "white"
                    font.pointSize: 18
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: cell.border.color = "white";
                    onExited: cell.border.color = "#00000000"
                    onClicked: {
                        lobby.userListModel.select(row);
                        if (lobby.canInvite(row))
                            inviteButton.setActive();
                        else
                            inviteButton.setInactive();
                    }
                }
            }
        }
    }

    MenuButton {
        text: "Join lobby"
        anchors {
            top: tableRectangle.bottom
            topMargin: 15
            left: tableRectangle.left
            leftMargin: 20
        }
        onPressed: {
            lobby.joinQueue()
        }
    }

    MenuButton {
        id: inviteButton

        text: "Invite"
        anchors {
            top: tableRectangle.bottom
            topMargin: 15
            right: tableRectangle.right
            rightMargin: 20
        }
        Component.onCompleted: {
            setInactive();
        }
    }

    Row {
        anchors {
            left: tableRectangle.right;
            leftMargin: 15;
            top: tableRectangle.top;
        }
        Text {
            text: "Users online: "
            font.pointSize: 24
            color: "white"
        }

        Text {
            text: onlineCount.toString()
            font.pointSize: 24
            color: "white"
        }
    }
}
