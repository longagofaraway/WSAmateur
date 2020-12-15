import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    id: thisItem

    property string mText

    z: 150
    width: 1200
    height: 100
    x: root.width / 2 - width / 2;
    y: root.height / 2 - height / 2;
    opacity: 0

    RadialGradient {
        anchors.fill: parent
        horizontalRadius: width
        verticalRadius: 80
        gradient: Gradient {
            GradientStop { position: 0.0; color: "white" }
            GradientStop { position: 0.5; color: "#00FFFFFF" }
        }
    }
    Text {
        anchors.centerIn: parent
        text: mText
        color: "black"
        font.pointSize: 25
        font.bold: true
    }

    NumberAnimation on opacity { to: 1; duration: 100 }
    SequentialAnimation {
        id: destroyAnim
        NumberAnimation { target: thisItem; id: anim; property: "opacity"; to: 0; duration: 100; }
        ScriptAction { script: gGame.destroyHelpText() }
    }

    function startDestroy() {
        destroyAnim.start();
    }
}
