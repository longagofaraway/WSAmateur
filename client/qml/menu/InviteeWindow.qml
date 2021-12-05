import QtQuick 2.0

Item {
    id: inviteeWindow

    property string name: ""
    signal acceptInvite()
    signal refuseInvite()

    visible: false

    MouseArea {
        anchors.fill: parent
        z: -1
        hoverEnabled: true
    }

    Rectangle {
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
                text: "User " + name
                color: "white"
                font.pointSize: 24
            }
            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "is inviting you to play"
                color: "white"
                font.pointSize: 24
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
                        refuseInvite();
                        inviteeWindow.visible = false;
                    }
                }
                MenuButton {
                    width: 220

                    text: "Accept"
                    onPressed: {
                        acceptInvite();
                        inviteeWindow.visible = false;
                    }
                }
            }
        }
    }

    function show(userName) {
        inviteeWindow.name = userName;
        inviteeWindow.visible = true;
    }

    function hide() {
        inviteeWindow.visible = false;
    }
}
