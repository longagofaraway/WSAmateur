import QtQuick 2.0

Item {
    id: errorWindow

    property string message

    MouseArea {
        anchors.fill: parent
        z: -1
        hoverEnabled: true
    }

    Rectangle {
        id: mainRectangle

        anchors.centerIn: parent

        width: mainWindow.width / 2.7
        height: mainColumn.implicitHeight * 1.2
        border.width: 2
        border.color: "white"
        color: "#E0000000"
        radius: 10

        Column {
            id: mainColumn
            anchors.centerIn: parent
            spacing: 20

            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: message
                color: "white"
                font.pointSize: 24
            }

            MenuButton {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                text: "Exit"
                onPressed: {
                    Qt.quit();
                }
            }
        }
    }
}
