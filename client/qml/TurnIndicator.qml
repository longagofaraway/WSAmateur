import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    id: thisItem

    property bool opponent
    property color mColor1: opponent ? "#2BFDFF" : "#FCDE01"
    property color mColor2: opponent ? "#002BFDFF" : "#00FCDE01"

    z: 150
    width: 1100
    height: 200
    x: root.width / 2 - width / 2;
    y: root.height / 2 - height / 2;
    scale: 0

    RadialGradient {
        anchors.fill: parent
        horizontalRadius: opponent ? width : 700
        verticalRadius: 100
        gradient: Gradient {
            GradientStop { position: 0.0; color: mColor1 }
            GradientStop { position: 0.5; color: mColor2 }
        }
    }
    Text {
        anchors.centerIn: parent
        text: opponent ? "Opponent's Turn" : "Your Turn"
        color: "white"
        font.pointSize: 60
        font.bold: true
    }

    function startAnimation() {
        show.start();
    }

    SequentialAnimation {
        id: show
        NumberAnimation { target: thisItem; property: "scale"; to: 1; duration: 300; }
        PauseAnimation { duration: 1000 }
        ParallelAnimation {
            NumberAnimation { target: thisItem; property: "scale"; to: 0; duration: 300; }
            NumberAnimation { target: thisItem; property: "y"; to: opponent ? 0 : root.height; duration: 300; }
        }
        ScriptAction {
            script:  {
                gGame.actionComplete();
                thisItem.destroy();
            }
        }
    }
}
