import QtQuick 2.0

Card {
    id: movingCard
    property int uniqueId
    property string code
    property bool opponent
    property bool isUiAction
    property bool isQmlAction: false
    property bool dontFinishAction: false
    property bool noDelete: false
    property string startZone
    property int startPos: 0
    property string targetZone
    property int targetPos: 0
    signal moveFinished()

    property real toX
    property real toY
    property real toScale: 1
    property real toRot: 0
    property string toImg: "cardback"

    z: 100

    function startAnimation() {
        if (!isQmlAction)
            setupMoveFromZone();
        if (!noDelete)
            removeCard();
        setupMoveToZone();
    }

    function removeCard() {
        let szone = gGame.getZone(startZone, opponent);
        if (startZone == "marker")
            szone.removeMarker(startPos);
        else
            szone.removeCard(startPos);
    }

    function setupMoveFromZone() {
        let szone = gGame.getZone(startZone, opponent);
        movingCard.x = szone.getXForCard(startPos);
        movingCard.y = szone.getYForCard(startPos);
        movingCard.rotation = szone.rotation;
        if (startZone === "hand" && opponent)
            movingCard.rotation += 180;
        if (startZone === "stock")
            movingCard.rotation += -90;
        else if (startZone === "level")
            movingCard.rotation += 90;
        else if (targetZone == "climax") {
            movingCard.rotation += -90;
            movingCard.y += -((root.cardHeight - root.cardWidth) / 2 + root.cardWidth);
        }
        if (startZone === "clock" || startZone === "level")
            movingCard.scale = szone.scaleForMovingCard();
    }

    function setupMoveToZone() {
        if (targetZone === "reveal") {
            toX = opponent ? root.width * 0.15 : root.width * 0.75;
            toY = opponent ? root.height * 0.3 : root.height * 0.6;
            let szone = gGame.getZone(startZone, opponent);
            let backX = szone.getXForCard(startPos);
            let backY = szone.getYForCard(startPos);
            if (opponent) {
                movingCard.mSource = "cardback";
                toImg = code;
                oppRevealAnim.backX = backX;
                oppRevealAnim.backY = backY;
                oppRevealAnim.start();
            } else {
                playerRevealAnim.backX = backX;
                playerRevealAnim.backY = backY;
                playerRevealAnim.start();
            }
            return;
        }

        let tzone = gGame.getZone(targetZone, opponent);
        if (targetPos == -1) {
            toX = tzone.getXForNewCard(targetPos);
            toY = tzone.getYForNewCard(targetPos);
        } else {
            toX = tzone.getXForCard(targetPos);
            toY = tzone.getYForCard(targetPos);
        }

        if (targetZone === "clock" || targetZone === "level") {
            toScale = tzone.scaleForMovingCard();
        }
        toRot = tzone.rotation;
        if (targetZone === "stage" && opponent)
            toRot += 180;
        if (targetZone === "stock")
            toRot += -90;
        else if (targetZone === "level")
            toRot += 90;
        else if (targetZone == "hand" && opponent)
            toRot += 180;
        if (toRot > 180)
            toRot -= 360;
        if (startZone === "deck") {
            movingCard.mSource = "cardback";
            movingCard.rotation = 0;
            if (code === "") {
                straightMove.start();
            } else {
                toImg = code;
                if (targetZone === "hand") drawingMove.start();
                else flippingMove.start();
            }
        } else {
            straightMove.start();
        }
    }

    function insertCard() {
        var zone = gGame.getZone(targetZone, opponent);
        if (targetZone == "marker")
            zone.addMarker(uniqueId, code, targetPos, startZone, startPos);
        else
            zone.addCard(uniqueId, code, targetPos, startZone, startPos);
        movingCard.visible = false;
    }

    function completeAction() {
        if (!isQmlAction) {
            if (!dontFinishAction) {
                if (isUiAction)
                    gGame.uiActionComplete();
                else
                    gGame.actionComplete();
            }
            moveFinished();
        } else {
            gGame.uiActionComplete();
            movingCard.destroy();
        }
    }

    function finishMove() {
        insertCard();
        gGame.getPlayer(opponent).cardInserted(startZone, targetZone);
        completeAction();
    }

    transform: Rotation {
        id: yRot
        origin.x: movingCard.width / 2
        origin.y: movingCard.height / 2
        angle: 0
        axis { x: 0; y: 1; z: 0 }
    }

    SequentialAnimation {
        id: straightMove
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: toX; duration: 300; }
            NumberAnimation { target: movingCard; property: "y"; to: toY; duration: 300; }
            NumberAnimation { target: movingCard; property: "scale"; to: toScale; duration: 300; }
            NumberAnimation { target: movingCard; property: "rotation"; to: toRot; duration: 300; }
        }
        ScriptAction { script: finishMove() }
    }

    /*SequentialAnimation {
        id: straightFlippingMove
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: toX; duration: 2000; }
            NumberAnimation { target: movingCard; property: "y"; to: toY; duration: 2000; }
            NumberAnimation { target: movingCard; property: "scale"; to: toScale; duration: 2000; }
            NumberAnimation { target: movingCard; property: "rotation"; to: -90; duration: 2000; }
            SequentialAnimation {
                NumberAnimation { target: yRot; property: "angle"; from: 0; to: -90; duration: 1000; }
                PropertyAction { target: movingCard; property: "mSource"; value: toImg }
                NumberAnimation { target: yRot; property: "angle"; from: -90; to: -180; duration: 1000; }
            }
        }
        ScriptAction { script: finishMove() }
    }*/
    SequentialAnimation {
        id: flippingMove
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: toX; duration: 200 }
            NumberAnimation { target: movingCard; property: "y"; to: toY; duration: 200 }
            SequentialAnimation {
                NumberAnimation { target: yRot; property: "angle"; from: 0; to: 90; duration: 100; }
                PropertyAction { target: movingCard; property: "mSource"; value: toImg }
                NumberAnimation { target: yRot; property: "angle"; from: -90; to: 0; duration: 100; }
            }
        }
        ScriptAction { script: { finishMove(); } }
    }

    ParallelAnimation {
        id: drawingMove
        NumberAnimation { target: movingCard; property: "x"; to: root.width * (opponent ? 0.2 : 0.75); duration: 200 }
        NumberAnimation { target: movingCard; property: "y"; to: root.height * (opponent ? 0.25 : 0.6); duration: 200 }
        SequentialAnimation {
            NumberAnimation { target: yRot; property: "angle"; from: 0; to: 90; duration: 100; }
            PropertyAction { target: movingCard; property: "mSource"; value: toImg }
            NumberAnimation { target: yRot; property: "angle"; from: -90; to: 0; duration: 100; }
            PauseAnimation { duration: 300 }
        }
        onStopped: toHandAnim.start();
    }

    SequentialAnimation {
        id: toHandAnim
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: toX; duration: 200 }
            NumberAnimation { target: movingCard; property: "y"; to: toY; duration: 200 }
        }
        ScriptAction { script: finishMove() }
    }

    SequentialAnimation {
        id: playerRevealAnim
        property real backX
        property real backY
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: toX; duration: 300; }
            NumberAnimation { target: movingCard; property: "y"; to: toY; duration: 300; }
        }
        PauseAnimation { duration: 500 }
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: playerRevealAnim.backX; duration: 300; }
            NumberAnimation { target: movingCard; property: "y"; to: playerRevealAnim.backY; duration: 300; }
        }
        ScriptAction { script: completeAction() }
    }

    SequentialAnimation {
        id: oppRevealAnim
        property real backX
        property real backY
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: toX; duration: 200 }
            NumberAnimation { target: movingCard; property: "y"; to: toY; duration: 200 }
            SequentialAnimation {
                NumberAnimation { target: yRot; property: "angle"; from: 0; to: 90; duration: 100; }
                PropertyAction { target: movingCard; property: "mSource"; value: toImg }
                NumberAnimation { target: yRot; property: "angle"; from: -90; to: 0; duration: 100; }
            }
        }
        PauseAnimation { duration: 800 }
        ParallelAnimation {
            NumberAnimation { target: movingCard; property: "x"; to: oppRevealAnim.backX; duration: 200 }
            NumberAnimation { target: movingCard; property: "y"; to: oppRevealAnim.backY; duration: 200 }
            SequentialAnimation {
                NumberAnimation { target: yRot; property: "angle"; from: 0; to: 90; duration: 100; }
                PropertyAction { target: movingCard; property: "mSource"; value: "cardback" }
                NumberAnimation { target: yRot; property: "angle"; from: -90; to: 0; duration: 100; }
            }
        }
        ScriptAction { script: completeAction() }
    }
}
