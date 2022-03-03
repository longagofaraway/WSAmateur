import QtQuick 2.0
import QtQuick.Dialogs 1.3

Item {
    id: chooseWindow
    signal fileChosen(string path)

    FileDialog {
        id: fileDialog

        title: "Please choose a file"
        folder: shortcuts.home
        nameFilters: [ "All files (*)" ]
        onAccepted: {
            chooseWindow.fileChosen(fileDialog.fileUrl);
        }
    }

    Rectangle {
        id: mainRectangle

        anchors.centerIn: parent

        width: mainWindow.width / 2.7
        height: mainWindow.height / 4.5
        border.width: 2
        border.color: "white"
        color: "#A0000000"
        radius: 10

        Column {
            anchors.centerIn: parent
            spacing: 20
            Text {
                id: parseErrorLabel
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Error parsing file"
                color: "red"
                font.pointSize: 16
                visible: false
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Specify a path to the file with card image links:"
                color: "white"
                font.pointSize: 20
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Visit <a href='https://discord.gg/6hRd2VZ3vG'>discord</a> for details"
                color: "white"
                font.pointSize: 16
                onLinkActivated: Qt.openUrlExternally(link)
            }

            MenuButton {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Choose file"
                onPressed: {
                    fileDialog.visible = true;
                }
            }
        }
    }

    function parseError() {
        parseErrorLabel.visible = true;
    }
}
