import QtQuick 2.0

import wsamateur.cardModel 1.0

Card {
    id: waitingRoom
    property bool opponent
    property bool hidden: false
    property CardModel mModel: innerModel
    Connections {
        target: mModel
        onCountChanged: {
            if (mModel.count == 0)
                waitingRoom.visible = false;
            else if (!waitingRoom.visible)
                waitingRoom.visible = true;
        }
    }

    visible: false

    x: {
        if (opponent)
            return root.width * 0.03;
        return root.width * 0.97 - root.cardWidth;
    }
    y: {
        if (opponent)
            return root.height * 0.25 - root.cardHeight;
        return root.height * 0.75;
    }
    z: 1

    function addCard(code) {
        waitingRoom.mSource = code;
        waitingRoom.mModel.addCard(code);
    }
    function removeCard(index) {
        waitingRoom.mModel.removeCard(index);
        if (handView.mModel.count > 0) {
            let modelIndex = handView.mModel.index(index, 0);
            waitingRoom.mSource = handView.mModel.data(modelIndex, CardModel.CodeRole);
        }
    }

    function getXForNewCard() { return waitingRoom.x; }
    function getYForNewCard() { return waitingRoom.y; }
    function getXForCard() { return waitingRoom.x; }
    function getYForCard() { return waitingRoom.y; }
}
