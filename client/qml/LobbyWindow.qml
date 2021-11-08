import QtQuick 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.15

import wsamateur 1.0

Lobby {
    id: lobby

    property real tableCellWidth: 500
    property real tableCellHeight: 40

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

    Rectangle {
        id: sideMenu

        property var activeGradient: Gradient {
            GradientStop { position: 0.0; color: "#60000000" }
            GradientStop { position: 0.3; color: "#50333333" }
            GradientStop { position: 0.5; color: "#50888888" }
            GradientStop { position: 0.7; color: "#50333333" }
            GradientStop { position: 1.0; color: "#60000000" }
        }
        property var inactiveGradient: Gradient {
            GradientStop { position: 0.0; color: "#50000000" }
            GradientStop { position: 1.0; color: "#50000000" }
        }
        property var hoverGradient: Gradient {
            GradientStop { position: 0.0; color: "#50000000" }
            GradientStop { position: 0.42; color: "#50000000" }
            GradientStop { position: 0.5; color: "#50777777" }
            GradientStop { position: 0.58; color: "#50000000" }
            GradientStop { position: 1.0; color: "#50000000" }
        }

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.width * 0.09

        color: "#50000000"

        Rectangle {
            id: topMenu
            anchors { left: parent.left; right: parent.right }
            y: parent.height * 0.1
            height: parent.height * 0.15
            gradient: sideMenu.activeGradient

            Text {
                anchors.centerIn: parent
                text: "Lobby"
                color: "white"
                font.pointSize: 36
            }
        }

        Rectangle {
            id: decksMenu
            anchors { left: parent.left; right: parent.right; top: topMenu.bottom }
            height: parent.height * 0.15
            gradient: sideMenu.inactiveGradient

            Text {
                anchors.centerIn: parent
                text: "Decks"
                color: "white"
                font.pointSize: 36
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: decksMenu.gradient = sideMenu.hoverGradient
                onExited: decksMenu.gradient = sideMenu.inactiveGradient
            }
        }
    }

    Rectangle {
        id: tableRectangle

        property real margin: 30

        anchors.verticalCenter: parent.verticalCenter
        x: lobby.width / 4
        color: "#50000000"
        width: tableCellWidth * lobby.gameListModel.columnCount()
        height: 500 + tableCellHeight + margin * 2

        HorizontalHeaderView {
            id: horizontalHeader
            width: contentWidth
            syncView: tableView

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

            model: lobby.gameListModel

            delegate: Rectangle {
                id: cell
                implicitWidth: tableCellWidth
                implicitHeight: tableCellHeight
                border.width: 1
                border.color: "#00000000"
                color: "#00000000"

                Text {
                    anchors.fill: parent
                    text: tableData
                    color: "white"
                    font.pointSize: 18
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                  anchors.fill: parent
                  hoverEnabled: true
                  onEntered: cell.border.color = "white";
                  onExited: cell.border.color = "#00000000"
                  onClicked: {
                    // print value from clicked cell
                    console.log("Clicked cell: ", model.tableData);
                  }

                  /*Item {
                      id: btn

                      anchors { right: parent.right; rightMargin: 15 }
                      anchors.verticalCenter: parent.verticalCenter
                      width: 89//231
                      height: 28//72
                      visible: false

                      Image {
                          id: btnNormal
                          anchors.fill: parent
                          source: "qrc:///resources/images/btn_menu_base"
                      }

                      MouseArea {
                          id: mouse
                          anchors.fill: parent
                          hoverEnabled: true
                          onPressed: btnNormal.source = "qrc:///resources/images/btn_menu_pressed"
                          onReleased: btnNormal.source = "qrc:///resources/images/btn_menu_hover"
                          onEntered: btnNormal.source = "qrc:///resources/images/btn_menu_hover"
                          onExited: btnNormal.source = "qrc:///resources/images/btn_menu_base"
                      }

                      Text {
                          anchors.centerIn: parent
                          text: "Invite"
                          color: "white"
                          font.pointSize: 18
                      }
                  }*/
                }
            }
        }
    }

    MenuButton {
        mText: "Join lobby"
        anchors {
            top: tableRectangle.bottom
            topMargin: 15
            left: tableRectangle.left
            leftMargin: 20
        }
    }

    MenuButton {
        mText: "Invite"
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
}
