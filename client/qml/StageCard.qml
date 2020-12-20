import QtQuick 2.12

Card {
    id: stageCard

    property bool dragActive: false

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

    function startAnimation(_x, _y) {
        aX.to = _x;
        aY.to = _y;
        moveAnim.start();
    }
}
