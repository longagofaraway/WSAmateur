import QtQuick 2.12

Card {
    id: topDeckCard

    property bool opponent: false
    property string img
    property string destination
    property int cardHandModelIndex
    property var onFinishedAction

    x: opponent ? opponentDeck.x : deck.x
    y: opponent ? opponentDeck.y : deck.y
    rotation: opponent ? 180 : 0
    source: "qrc:///resources/images/cardback"

    function startPlayerAnimation(dest, _x, _y) {
        destination = dest;
        aHandX.to = _x;
        aHandY.to = _y;
        revealAnim.start();
    }
    function startOpponentAnimation(dest, _x, _y) {
        destination = dest;
        oppHandPoint.x = _x - root.cardWidth / 2;
        oppHandPoint.y = _y - root.cardHeight / 2;
        opponentDraw.start();
    }
    function startToStockAnim(dest, _x, _y, finishAction) {
        destination = dest;
        aStockX.to = _x;
        aStockY.to = _y;
        revealAnim.start();
        onFinishedAction = finishAction;
    }

    transform: Rotation {
        id: yRot
        origin.x: topDeckCard.width / 2
        origin.y: topDeckCard.height / 2
        angle: 0
        axis { x: 0; y: 1; z: 0 }
    }

    ParallelAnimation {
        id: revealAnim
        running: false

        NumberAnimation { target: topDeckCard; property: "x"; to: root.width * (opponent ? 0.2 : 0.75); duration: 200 }
        NumberAnimation { target: topDeckCard; property: "y"; to: root.height * (opponent ? 0.25 : 0.6); duration: 200 }
        SequentialAnimation {
            NumberAnimation {
                target: yRot;
                property: "angle";
                from: 0; to: 90;
                duration: 100;
            }
            PropertyAction {
                target: topDeckCard
                property: "source"
                value: topDeckCard.img
            }
            NumberAnimation {
                target: yRot;
                property: "angle";
                from: -90; to: 0;
                duration: 100;
            }
            PauseAnimation { duration: 300 }
        }

        onStopped: {
            if (destination === "hand")
                toHandAnim.start();
            else if (destination === "stock")
                toStockAnim.start();
        }
    }

    ParallelAnimation {
        id: toHandAnim
        NumberAnimation { id: aHandX; target: topDeckCard; property: "x"; duration: 200 }
        NumberAnimation { id: aHandY; target: topDeckCard; property: "y"; duration: 200 }
        onStopped: {
            handView.model.model.setProperty(cardHandModelIndex, "src", img);
            topDeckCard.destroy();
        }
    }

    ParallelAnimation {
        id: toStockAnim
        SequentialAnimation {
            NumberAnimation {
                target: yRot;
                property: "angle";
                from: 0; to: 90;
                duration: 150;
            }
            PropertyAction {
                target: topDeckCard
                property: "source"
                value: "qrc:///resources/images/cardback"
            }
            NumberAnimation {
                target: yRot;
                property: "angle";
                from: -90; to: 0;
                duration: 150;
            }
        }
        NumberAnimation { id: aStockX; target: topDeckCard; property: "x"; duration: 300 }
        NumberAnimation { id: aStockY; target: topDeckCard; property: "y"; duration: 300 }
        NumberAnimation { target: topDeckCard; property: "rotation"; to: 90 * (opponent ? 1 : -1); duration: 300 }
        onStopped: {
            onFinishedAction();
            topDeckCard.destroy();
        }
    }

    PathAnimation {
        id: opponentDraw
        running: false

        duration: 500
        target: topDeckCard

        path: Path {
            startX: topDeckCard.x
            startY: topDeckCard.y
            PathQuad { id: oppHandPoint; controlX: root.width * 0.3; controlY: root.height * 0.2 }
        }
        onStopped: {
            oppHandView.model.model.setProperty(cardHandModelIndex, "src", "qrc:///resources/images/cardback");
            topDeckCard.destroy();
        }
    }
}
