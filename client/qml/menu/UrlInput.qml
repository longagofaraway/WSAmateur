import QtQuick 2.0

Item {
    id: modalUrl

    signal add(string url)
    signal cancel()

    visible: false

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
                id: errorReason
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: ""
                color: "red"
                font.pointSize: 16
                visible: false
            }

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
                    id: addButton
                    width: 220

                    text: "Add"
                    onPressed: {
                        addButton.active = false;
                        modalUrl.add(urlInput.text);
                    }
                }
            }
        }
    }

    function show() {
        urlInput.clear();
        errorReason.visible = false;
        modalUrl.visible = true;
    }

    function setError(reason) {
        addButton.active = true;
        errorReason.text = reason;
        urlInput.setError();
        errorReason.visible = true;
    }

    function activateAddButton() {
        addButton.active = true;
    }
}
