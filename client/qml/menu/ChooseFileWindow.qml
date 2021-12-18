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

        width: mainWindow.width / 3.2
        height: mainWindow.height / 4.8
        border.width: 2
        border.color: "white"
        color: "#A0000000"
        radius: 10

        Column {
            anchors.centerIn: parent
            spacing: 20
            Text {
                id: parseErrorLabel
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "Error parsing file"
                color: "red"
                font.pointSize: 16
                visible: false
            }
            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "Specify a file with card image links:"
                color: "white"
                font.pointSize: 24
            }
            BasicTextInput {
                id: urlInput

                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                width: mainRectangle.width * 0.7
            }
            MenuButton {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

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
