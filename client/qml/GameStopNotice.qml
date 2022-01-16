import QtQuick 2.0
import QtGraphicalEffects 1.12

MouseArea {
    id: rootLabel

    property string reason
    property string exitDestination

    width: gGame.width
    height: gGame.height
    z: 400

    onClicked: {
        if (exitDestination === "start")
            wsApp.startWindow();
        else if (exitDestination === "lobby")
            gGame.quitGame();
    }

    Item {
        id: label

        property color mColor1: "#2BFDFF"
        property color mColor2: "#002BFDFF"

        width: 1100
        height: 200
        x: root.width / 2 - width / 2;
        y: root.height / 2 - height / 2;
        scale: 0

        RadialGradient {
            anchors.fill: parent
            horizontalRadius: 700
            verticalRadius: 100
            gradient: Gradient {
                GradientStop { position: 0.0; color: label.mColor1 }
                GradientStop { position: 0.5; color: label.mColor2 }
            }
        }
        Text {
            anchors.centerIn: parent
            text: rootLabel.reason
            color: "white"
            font.pointSize: 30
            font.bold: true
        }

        NumberAnimation { id: show; target: label; property: "scale"; to: 1; duration: 400; }
    }

    function startAnimation() {
        show.start();
    }
}
