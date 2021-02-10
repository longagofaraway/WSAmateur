import QtQuick 2.0

Text {
    id: attrText
    font.pointSize: 12
    font.family: "Aprikas Black Demo"

    SequentialAnimation {
        id: anim
        ParallelAnimation {
            NumberAnimation {
                target: attrText
                property: "opacity"
                to: 0
                duration: 300
            }
            NumberAnimation {
                target: attrText
                property: "scale"
                to: 3
                duration: 300
            }
        }
        ScriptAction { script: attrText.destroy(); }
    }

    function startAnim() { anim.start(); }
}
