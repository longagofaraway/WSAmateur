import QtQuick 2.0

import wsamateur.cardModel 1.0

Card {
    id: deck
    property bool opponent
    property CardModel mModel: innerModel

    source: "image://imgprov/cardback"
    rotation: opponent ? 180 : 0

    x: {
        if (opponent)
            return root.width * 0.05;
        return root.width * 0.95 - root.cardWidth;
    }
    y: {
        if (opponent)
            return root.height * 0.47 - root.cardHeight;
        return root.height * 0.53;
    }
    z: 1

    function addCard(code) {
        deck.mModel.addCard(code);
    }

    function getXForNewCard() { return deck.x; }
    function getYForNewCard() { return deck.y; }
    function getXForCard() { return deck.x; }
    function getYForCard() { return deck.y; }
}
