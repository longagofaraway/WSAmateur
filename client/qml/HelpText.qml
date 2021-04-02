import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    id: thisItem

    property string mText
    property string mHelpText

    z: 150
    width: 1200
    height: 100
    x: root.width / 2 - width / 2;
    y: root.height / 2 - height / 2;
    opacity: 0

    RadialGradient {
        anchors.fill: parent
        horizontalRadius: width
        verticalRadius: 100
        gradient: Gradient {
            GradientStop { position: 0.0; color: "white" }
            GradientStop { position: 0.5; color: "#00FFFFFF" }
        }
    }
    Text {
        id: mainText
        anchors.centerIn: parent
        width: 800
        text: mText
        color: "black"
        font.pointSize: 25
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        fontSizeMode: Text.HorizontalFit
    }
    Text {
        anchors.top: mainText.bottom
        anchors.horizontalCenter: mainText.horizontalCenter
        text: mHelpText
        color: "black"
        font.pointSize: 12
        font.bold: false
        horizontalAlignment: Text.AlignHCenter
    }

    NumberAnimation on opacity { to: 1; duration: 100 }
    SequentialAnimation {
        id: destroyAnim
        alwaysRunToEnd: true
        NumberAnimation { target: thisItem; id: anim; property: "opacity"; to: 0; duration: 100; }
        ScriptAction { script: gGame.destroyHelpText() }
    }

    function startDestroy() {
        destroyAnim.start();
    }
}
