import QtQuick 2.0

import wsamateur 1.0

Card {
    id: deck
    property bool opponent
    property bool hidden: true
    property CardModel mModel: innerModel
    property bool mGlow: false
    Connections {
        target: mModel
        function onCountChanged() {
            if (mModel.count == 0)
                deck.visible = false;
            else if (mModel.count > 0 && !deck.visible)
                deck.visible = true;
        }
    }

    mSource: "cardback"
    rotation: opponent ? 180 : 0

    x: {
        if (opponent)
            return root.width * 0.03;
        return root.width * 0.97 - root.cardWidth;
    }
    y: {
        if (opponent)
            return root.height * 0.38 - root.cardHeight;
        return root.height * 0.62;
    }
    z: 1

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: deckOverlay.opacity = 1
        onExited: deckOverlay.opacity = 0
        onClicked: {
            if (mGlow) {
                gGame.player.lookOrRevealTopDeck();
            }
        }
    }

    Rectangle {
        id: deckOverlay
        anchors.fill: parent
        color: "#80000000"
        opacity: 0

        rotation: opponent ? 180 : 0

        Behavior on opacity { NumberAnimation { duration: 200 } }

        Row {
            anchors.centerIn: deckOverlay
            spacing: 5

            Text {
                text: String(mModel.count)
                color: "white"
                font.pointSize: 21
                font.family: "Souvenir LT"
            }
            Image {
                source: "qrc:///resources/images/deckCount"
                width: 32 * 0.8
                height: 44 * 0.8
            }
        }
    }

    CardGlow {
        glow: mGlow
    }

    function addCard(code) { gGame.getPlayer(opponent).addCard(code, "deck"); }
    function removeCard(index) { deck.mModel.removeCard(index); }

    function getXForNewCard() { return deck.x; }
    function getYForNewCard() { return deck.y; }
    function getXForCard() { return deck.x; }
    function getYForCard() { return deck.y; }
}
