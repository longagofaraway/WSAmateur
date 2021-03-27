import QtQuick 2.12
import QtQml 2.12

import wsamateur 1.0

Card {
    id: waitingRoom
    property bool opponent
    property bool hidden: false
    property CardModel mModel: innerModel
    property CardsView mView: null
    Connections {
        target: mModel
        function onCountChanged() {
            if (mModel.count == 0) {
                waitingRoom.visible = false;
                mView.visible = false;
            } else if (!waitingRoom.visible) {
                waitingRoom.visible = true;
            }
            climaxCountText.text = mModel.climaxCount();
            cardCountText.text = mModel.nonClimaxCount();
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
            return root.height * 0.20 - root.cardHeight;
        return root.height * 0.80;
    }
    z: 1

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: wrOverlay.opacity = 1
        onExited: wrOverlay.opacity = 0
        onClicked: toggleView()
    }

    Rectangle {
        id: wrOverlay
        anchors.fill: parent
        color: "#80000000"
        opacity: 0

        Behavior on opacity { NumberAnimation { duration: 200 } }

        Column {
            id: thisColumn
            anchors.centerIn: wrOverlay
            spacing: 10

            Row {
                anchors.horizontalCenter: thisColumn.horizontalCenter
                spacing: 5

                Text {
                    id: cardCountText
                    text: "0"
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
            Row {
                anchors.horizontalCenter: thisColumn.horizontalCenter
                spacing: 5

                Text {
                    id: climaxCountText
                    text: "0"
                    color: "white"
                    font.pointSize: 21
                    font.family: "Souvenir LT"
                }
                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:///resources/images/wrClimaxCount"
                    width: 41 * 0.8
                    height: 29 * 0.8
                }
            }
        }
    }

    function toggleView() { (mView === null) ? openView(true) : openView(!mView.visible); }

    function openView(open) {
        if (mView === null) {
            let comp = Qt.createComponent("CardsView.qml");
            mView = comp.createObject(gGame);
            mView.mModel = mModel;
            if (waitingRoom.opponent) {
                mView.anchors.left = waitingRoom.right;
                mView.y = waitingRoom.y;
            } else {
                mView.anchors.right = waitingRoom.left;
            }
            mView.mOpponent = opponent;
            mView.closeSignal.connect(() => mView.visible = !mView.visible);
        }
        mView.visible = open;
    }

    function addCard(code) {
        waitingRoom.mSource = code;
        gGame.getPlayer(opponent).addCard(code, "wr");
    }
    function removeCard(index) {
        waitingRoom.mModel.removeCard(index);
        if (waitingRoom.mModel.count > 0) {
            let modelIndex = waitingRoom.mModel.index(waitingRoom.mModel.count - 1, 0);
            waitingRoom.mSource = waitingRoom.mModel.data(modelIndex, CardModel.CodeRole);
        }
    }

    function getXForNewCard() { return waitingRoom.x; }
    function getYForNewCard() { return waitingRoom.y; }
    function getXForCard() { return waitingRoom.x; }
    function getYForCard() { return waitingRoom.y; }
}
