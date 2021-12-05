import QtQuick 2.0

Item {
    id: inviterWindow

    signal cancelInvite()

    visible: false

    MouseArea {
        anchors.fill: parent
        z: -1
        hoverEnabled: true
    }

    Timer {
        id: inviteTimer

        property int inviteCountdown: 0
        interval: 1000; running: false; repeat: true
        onTriggered: {
            if (inviteCountdown == 0) {
                running = false;
                cancelInvite();
                inviterWindow.visible = false;
            }
            inviteCountdown -= 1;
            timerText.text = inviteCountdown.toString();
        }

        function resetTimer() {
            inviteCountdown = 10;
            inviteTimer.running = true;
            timerText.text = inviteCountdown.toString();
        }

        function stopTimer() {
            inviteTimer.running = false;
            inviterWindow.visible = false;
        }
    }

    Rectangle {
        anchors.centerIn: parent

        width: mainWindow.width / 4
        height: mainWindow.height / 6
        border.width: 2
        border.color: "white"
        color: "#A0000000"
        radius: 10

        Column {
            anchors.centerIn: parent
            spacing: 20
        Row {
            id: countdownRow
            Text {
                text: "Waiting for a reply... "
                color: "white"
                font.pointSize: 24
            }
            Text {
                id: timerText
                text: "0"
                color: "white"
                font.pointSize: 24
            }
        }

        MenuButton {
            anchors {
                horizontalCenter: countdownRow.horizontalCenter
            }

            width: 220

            text: "Cancel"
            onPressed: {
                inviteTimer.stopTimer();
                cancelInvite();
            }
        }
        }
    }

    function startCountdown() {
        inviterWindow.visible = true;
        inviteTimer.resetTimer();
    }

    function hide() {
        inviteTimer.stopTimer();
    }
}
