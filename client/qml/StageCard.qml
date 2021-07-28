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
    property int level: 0
    property string cardState: "Standing"
    property string cardType

    Drag.active: dragActive
    Drag.hotSpot.x: root.cardWidth / 2
    Drag.hotSpot.y: root.cardHeight
    rotation: {
        if (cardState === "Standing")
            return 0;
        if (cardState === "Rested")
            return -90;
        if (cardState === "Reversed")
            return -180;
    }

    states: State {
        when: stageCard.dragActive
        PropertyChanges { target: stageCard; z: 100 }
    }

    Rectangle {
        id: powerRect

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
        id: soulRect

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
    Rectangle {
        id: levelRect

        width: levelText.contentWidth + 10
        height: levelText.contentHeight + 6
        anchors.top: stageCard.top
        anchors.left: stageCard.left
        color: "#E0FFF4F4"
        radius: 8
        Text {
            id: levelText
            anchors.centerIn: parent
            text: level.toString()
            font.pointSize: 12
            font.family: "Aprikas Black Demo"
        }
    }

    Behavior on rotation { NumberAnimation { duration: 150 } }

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

    function powerChangeAnim() {
        let comp = Qt.createComponent("AttributeChangeAnimation.qml");
        let text = comp.createObject(powerRect);
        text.text = power.toString();
        text.anchors.centerIn = powerRect;
        text.startAnim();
    }

    function soulChangeAnim() {
        let comp = Qt.createComponent("AttributeChangeAnimation.qml");
        let text = comp.createObject(soulRect);
        text.text = soul.toString();
        text.anchors.verticalCenter = soulRect.verticalCenter;
        text.anchors.right = soulRect.right;
        text.anchors.rightMargin = 3;
        text.startAnim();
    }

    function levelChangeAnim() {
        let comp = Qt.createComponent("AttributeChangeAnimation.qml");
        let text = comp.createObject(levelRect);
        text.text = level.toString();
        text.anchors.centerIn = levelRect;
        text.startAnim();
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
