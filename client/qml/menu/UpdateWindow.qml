import QtQuick 2.0
import QtQuick.Controls 2.15

Item {
    id: updateWindow

    signal startUpdate()

    Rectangle {
        id: mainRectangle

        anchors.centerIn: parent

        width: mainWindow.width / 3.1
        height: mainWindow.height / 5.3
        border.width: 2
        border.color: "white"
        color: "#A0000000"
        radius: 10

        Column {
            id: confirmation
            anchors.centerIn: parent
            spacing: 20
            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "Update is needed. Click OK to download"
                color: "white"
                font.pointSize: 24
            }
            Row {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                spacing: 20
                MenuButton {
                    text: "Exit"
                    onPressed: {
                        Qt.quit();
                    }
                }
                MenuButton {
                    text: "OK"
                    onPressed: {
                        startUpdate();
                        confirmation.visible = false;
                        downloading.visible = true;
                    }
                }
            }
        }

        Column {
            id: downloading

            anchors.centerIn: parent
            visible: false
            spacing: 20
            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "Downloading..."
                color: "white"
                font.pointSize: 24
            }

            ProgressBar {
                id: progressBar
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    function setProgress(bytesRead, totalBytes) {
        progressBar.to = totalBytes;
        progressBar.value = bytesRead;
    }
}
