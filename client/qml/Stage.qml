import QtQuick 2.0

import wsamateur.cardModel 1.0

Item {
    id: stage

    property bool opponent
    property bool hidden: false
    property bool mDragEnabled: false
    property CardModel mModel: innerModel
    signal switchPositions(int from, int to)
    property var mPositions: [{ x: stage1.x, y: stage1.y },
                              { x: stage2.x, y: stage2.y },
                              { x: stage3.x, y: stage3.y },
                              { x: stage4.x, y: stage4.y },
                              { x: stage5.x, y: stage5.y }]
    property var mStagePlaces: [stage1, stage2, stage3, stage4, stage5]

    width: root.width; height: root.height

    StagePlace {
        id: stage1
        mIndex: 0
        x: root.width / 2 - root.cardWidth / 2 - root.width * 0.14;
    }
    StagePlace {
        id: stage2
        mIndex: 1
        x: root.width / 2 - root.cardWidth / 2;
    }
    StagePlace {
        id: stage3
        mIndex: 2
        x: root.width / 2 - root.cardWidth / 2 + root.width * 0.14;
    }
    StagePlace {
        id: stage4
        mIndex: 3
        x: root.width / 2 - root.cardWidth / 2 - root.width * 0.07;
    }
    StagePlace {
        id: stage5
        mIndex: 4
        x: root.width / 2 - root.cardWidth / 2 + root.width * 0.07;
    }

    function getXForNewCard(pos) { return mPositions[pos].x; }
    function getYForNewCard(pos) { return mPositions[pos].y; }
    function getXForCard(pos) { return mPositions[pos].x; }
    function getYForCard(pos) { return mPositions[pos].y; }
    function addCard(code, pos) { mStagePlaces[pos].setCard(code); }
}
