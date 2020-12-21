import QtQuick 2.12
import QtGraphicalEffects 1.12

Card {
    id: stageCard

    property bool dragActive: false
    property bool glowEnabled: false
    property int index

    Drag.active: dragActive
    Drag.hotSpot.x: root.cardWidth / 2
    Drag.hotSpot.y: root.cardHeight
    states: State {
        when: stageCard.dragActive
        PropertyChanges { target: stageCard; z: 100 }
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
        color: "#FFFFFF"
        cornerRadius: 0
        glowRadius: 10
        visible: glowEnabled
    }

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
