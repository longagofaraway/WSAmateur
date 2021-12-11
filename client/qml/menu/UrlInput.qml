import QtQuick 2.0

Item {
    id: modalUrl

    signal add(string url)
    signal cancel()

    visible: true

    MouseArea {
        anchors.fill: parent
        z: -1
        hoverEnabled: true
    }

    Rectangle {
        id: mainRectangle

        anchors.centerIn: parent

        width: mainWindow.width / 3
        height: mainWindow.height / 4.6
        border.width: 2
        border.color: "white"
        color: "#A0000000"
        radius: 10

        Column {
            anchors.centerIn: parent
            spacing: 20

            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "Paste EncoreDecks link:"
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

            Row {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                spacing: 20

                MenuButton {
                    width: 220

                    text: "Cancel"
                    onPressed: {
                        modalUrl.visible = false;
                        modalUrl.cancel();
                    }
                }
                MenuButton {
                    width: 220

                    text: "Add"
                    onPressed: {
                        modalUrl.add(urlInput.text);
                        modalUrl.visible = false;
                    }
                }
            }
        }
    }

    function show() {
        urlInput.clear();
        modalUrl.visible = true;
    }
}
