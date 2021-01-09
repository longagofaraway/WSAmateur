import QtQuick 2.0
import QtGraphicalEffects 1.12

Item {
    property bool glow: false
    property bool selected: false

    anchors.fill: parent

    RectangularGlow {
        id: cardGlow
        anchors.fill: parent
        z: -1
        color: selected ? "#FCDE01" : "#2BFDFF"
        cornerRadius: 0
        glowRadius: 10
        visible: glow || selected
    }
    SequentialAnimation {
        running: cardGlow.visible
        loops: 1000
        NumberAnimation {
            target: cardGlow
            property: "spread"
            from: 0
            to: 0.1
            duration: 2000
        }
        NumberAnimation {
            target: cardGlow
            property: "spread"
            from: 0.1
            to: 0
            duration: 2000
        }
    }
}
