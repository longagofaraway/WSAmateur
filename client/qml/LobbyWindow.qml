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
            to: "#B0000000"
            duration: 1000
        }
    }

    HorizontalHeaderView {
        id: horizontalHeader
        width: contentWidth
        syncView: tableView

        anchors {
            bottom: tableRectangle.top
            left: tableRectangle.left
            right: tableRectangle.right
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
            }
        }
    }

    Rectangle {
        id: tableRectangle

        anchors.centerIn: parent
        color: "#B0000000"
        width: 1000
        height: 500
    TableView {
        id: tableView

        anchors.fill: parent
        clip: true

        ScrollBar.vertical: ScrollBar {}

        model: lobby.gameListModel

        delegate: Rectangle {
            implicitWidth: tableCellWidth
            implicitHeight: tableCellHeight
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
              onClicked: {
                // print value from clicked cell
                console.log("Clicked cell: ", model.tableData);
              }
            }
        }
    }
    }
}
