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
}
