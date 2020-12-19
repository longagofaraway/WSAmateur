import QtQuick 2.0

import wsamateur.cardModel 1.0

Item {
    id: stage

    property bool opponent
    property CardModel mModel: innerModel

    width: root.width; height: root.height

    StagePlace {
        mIndex: 0
        x: root.width / 2 - root.cardWidth / 2 - root.width * 0.14;
    }
    StagePlace {
        mIndex: 1
        x: root.width / 2 - root.cardWidth / 2;
    }
    StagePlace {
        mIndex: 2
        x: root.width / 2 - root.cardWidth / 2 + root.width * 0.14;
    }
    StagePlace {
        mIndex: 3
        x: root.width / 2 - root.cardWidth / 2 - root.width * 0.07;
    }
    StagePlace {
        mIndex: 4
        x: root.width / 2 - root.cardWidth / 2 + root.width * 0.07;
    }
}
