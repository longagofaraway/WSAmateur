import QtQuick 2.0

Item {
    id: chooseWindow
    signal fileChosen(string path)

    /* Doesn't work for some reason
    FileDialog {
        id: fileDialog

        title: "Please choose a file"
        folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
        nameFilters: [ "Json files (*.json)", "All files (*)" ]
        onAccepted: {
            console.log('what');
            console.log(fileDialog.fileUrl);
            console.log(fileDialog.fileUrls);
            chooseWindow.fileChosen(fileDialog.fileUrl);
        }
        onRejected: {
            console.log('rejected');
        }
    }*/

    Rectangle {
        id: mainRectangle

        anchors.centerIn: parent

        width: mainWindow.width / 2.7
        height: mainWindow.height / 4.0
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
                text: "Visit <a href='https://discord.gg/K5jE8bmP'>discord</a> for details"
                color: "white"
                font.pointSize: 16
                onLinkActivated: Qt.openUrlExternally(link)
            }

            BasicTextInput {
                id: urlInput

                anchors.horizontalCenter: parent.horizontalCenter

                width: mainRectangle.width * 0.7
            }
            MenuButton {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "OK"
                onPressed: {
                    fileChosen(urlInput.text);
                }
            }
        }
    }

    function parseError() {
        parseErrorLabel.visible = true;
    }
}
