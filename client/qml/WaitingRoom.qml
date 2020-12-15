import QtQuick 2.0

import wsamateur.cardModel 1.0

Card {
    id: waitingRoom
    property bool opponent
    property CardModel mModel: innerModel

    x: {
        if (opponent)
            return root.width * 0.05;
        return root.width * 0.95 - root.cardWidth;
    }
    y: {
        if (opponent)
            return root.height * 0.25 - root.cardHeight;
        return root.height * 0.75;
    }
    z: 1

    function addCard(code) {
        waitingRoom.source = "image://imgprov/" + code;
        waitingRoom.mModel.addCard(code);
    }

    function getXForNewCard() { return waitingRoom.x; }
    function getYForNewCard() { return waitingRoom.y; }
    function getXForCard() { return waitingRoom.x; }
    function getYForCard() { return waitingRoom.y; }
}