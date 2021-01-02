import QtQuick 2.12
import QtGraphicalEffects 1.12

Card {
    id: stageCard

    property bool dragActive: false
    property bool cardOverGlow: false
    property bool glow: false
    property bool selected: false
    property int index
    property int power: 9000
    property int soul: 1

    Drag.active: dragActive
    Drag.hotSpot.x: root.cardWidth / 2
    Drag.hotSpot.y: root.cardHeight
    states: State {
        when: stageCard.dragActive
        PropertyChanges { target: stageCard; z: 100 }
    }

    Rectangle {
        width: powerText.contentWidth + 10
        height: powerText.contentHeight + 6
        anchors.bottom: stageCard.bottom
        anchors.left: stageCard.left
        color: "#E0FFF4F4"
        radius: 8
        Text {
            id: powerText
            anchors.centerIn: parent
            text: power.toString()
            font.pointSize: 12
            font.family: "Aprikas Black Demo"
        }
    }
    Rectangle {
        width: soulImg.width + soulText.contentWidth + 10
        height: powerText.contentHeight + 6
        anchors.right: stageCard.right
        anchors.bottom: stageCard.bottom
        color: "#E0FFF4F4"
        radius: 8
        Text {
            id: soulText
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 3
            text: soul.toString()
            font.pointSize: 12
            font.family: "Aprikas Black Demo"
        }
        Image {
            id: soulImg
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: soulText.left
            anchors.rightMargin: 3
            source: "qrc:///resources/images/soul"
            width: 15
            height: width / 49 * 58
        }
    }

    Behavior on rotation { NumberAnimation { duration: 150 } }
    SequentialAnimation {
        id: tapAnim
        NumberAnimation { target: stageCard; property: "rotation"; to: -90; duration: 150 }
        PauseAnimation { duration: 500 }
        ScriptAction { script: gGame.uiActionComplete() }
    }

    ParallelAnimation {
        id: moveAnim
        NumberAnimation { id: aX; target: stageCard; property: "x"; duration: 200 }
        NumberAnimation { id: aY; target: stageCard; property: "y"; duration: 200 }
    }

    SequentialAnimation {
        id: dropAnim
        ParallelAnimation {
            NumberAnimation { id: dropX; target: stageCard; property: "x"; duration: 150; }
            NumberAnimation { id: dropY; target: stageCard; property: "y"; duration: 150; }
        }
    }

    RectangularGlow {
        id: cardGlow
        anchors.fill: stageCard
        z: -1
        color: getGlowColor()
        cornerRadius: 0
        glowRadius: 10
        visible: cardOverGlow || glow || selected
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

    function tap() {
        gGame.startUiAction();
        tapAnim.start();
    }
    function getGlowColor() { return cardOverGlow ? "#FFFFFF" : (selected ? "#FCDE01" : "#2BFDFF"); }
    function onCardEntered() { cardOverGlow = true; }
    function onCardExited() { cardOverGlow = false; }

    function startAnimation(_x, _y) {
        aX.to = _x;
        aY.to = _y;
        moveAnim.start();
    }

    function startDropAnim(_x, _y) {
        dropX.to = _x;
        dropY.to = _y;
        dropAnim.start();
    }
}
