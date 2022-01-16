import QtQuick 2.0
import QtGraphicalEffects 1.12

MouseArea {
    id: rootEnd

    property bool victory

    width: gGame.width
    height: gGame.height
    z: 400

    onClicked: gGame.quitGame()

    Item {
        id: endLabel

        property color mColor1: rootEnd.victory ? "#FCDE01" :"#2BFDFF"
        property color mColor2: rootEnd.victory ? "#00FCDE01" : "#002BFDFF"

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
                GradientStop { position: 0.0; color: endLabel.mColor1 }
                GradientStop { position: 0.5; color: endLabel.mColor2 }
            }
        }
        Text {
            anchors.centerIn: parent
            text: rootEnd.victory ? "You won" : "Wasted"
            color: "white"
            font.pointSize: 60
            font.bold: true
        }

        NumberAnimation { id: show; target: endLabel; property: "scale"; to: 1; duration: 400; }
    }

    function startAnimation() {
        show.start();
    }
}
