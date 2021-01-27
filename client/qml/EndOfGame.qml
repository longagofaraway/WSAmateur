import QtQuick 2.0
import QtGraphicalEffects 1.12

Item {
    id: thisItem

    property bool victory
    property color mColor1: victory ? "#FCDE01" :"#2BFDFF"
    property color mColor2: victory ? "#00FCDE01" : "#002BFDFF"

    z: 200
    width: 1100
    height: 200
    x: root.width / 2 - width / 2;
    y: root.height / 2 - height / 2;
    scale: 0

    MouseArea {
        width: gGame.width
        height: gGame.height

        onClicked: Qt.quit()
    }

    RadialGradient {
        anchors.fill: parent
        horizontalRadius: 700
        verticalRadius: 100
        gradient: Gradient {
            GradientStop { position: 0.0; color: mColor1 }
            GradientStop { position: 0.5; color: mColor2 }
        }
    }
    Text {
        anchors.centerIn: parent
        text: victory ? "You won" : "Wasted"
        color: "white"
        font.pointSize: 60
        font.bold: true
    }

    function startAnimation() {
        show.start();
    }

    NumberAnimation { id: show; target: thisItem; property: "scale"; to: 1; duration: 400; }
}
