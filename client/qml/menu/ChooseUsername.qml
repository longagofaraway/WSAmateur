import QtQuick 2.0

Item {
    id: chooseWindow
    signal usernameChosen(string username)

    Rectangle {
        id: mainRectangle

        anchors.centerIn: parent

        width: mainWindow.width / 3.2
        height: mainWindow.height / 5
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
                text: "Choose a nickname:"
                color: "white"
                font.pointSize: 24
            }
            Text {
                id: errorText
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "User name must be at least 2 characters"
                color: "red"
                font.pointSize: 16
                visible: false
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
                    if (urlInput.text.length <= 1) {
                        errorText.visible = true;
                        return;
                    }
                    usernameChosen(urlInput.text);
                }
            }
        }
    }
}
