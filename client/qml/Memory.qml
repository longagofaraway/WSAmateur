import QtQuick 2.15

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

Card {
    id: memory
    property bool opponent
    property bool hidden: false
    property CardModel mModel: innerModel
    property CardsView mView: null
    Connections {
        target: mModel
        function onCountChanged() {
            if (mModel.count == 0) {
                memory.visible = false;
                mView.visible = false;
            } else if (!memory.visible) {
                memory.visible = true;
            }
        }
    }

    scale: 0.8
    visible: false
    x: {
        if (opponent)
            return root.width * 0.03;
        return root.width * 0.97 - root.cardWidth;
    }
    y: {
        if (opponent)
            return root.height * 0.53 - root.cardHeight;
        return root.height * 0.47;
    }
    z: 1
    rotation: -90

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: memoryOverlay.opacity = 1
        onExited: memoryOverlay.opacity = 0
        onClicked: toggleView()
    }

    Rectangle {
        id: memoryOverlay
        anchors.fill: parent
        color: "#80000000"
        opacity: 0

        Behavior on opacity { NumberAnimation { duration: 200 } }

        Row {
            anchors.centerIn: memoryOverlay
            spacing: 5
            rotation: 90

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

    function toggleView() { (mView === null) ? openView(true) : openView(!mView.visible); }

    function openView(open) {
        if (mView === null) {
            console.log("asdf");
            let comp = Qt.createComponent("CardsView.qml");
            mView = comp.createObject(gGame);
            mView.mModel = mModel;
            mView.mZoneName = "Memory";
            if (memory.opponent) {
                mView.anchors.left = memory.right;
                mView.y = memory.y;
            } else {
                mView.anchors.right = memory.left;
            }
            mView.mOpponent = opponent;
            mView.closeSignal.connect(() => mView.visible = !mView.visible);
            return;
        }
        mView.visible = open;
    }

    function addCard(code) {
        memory.mSource = code;
        memory.mModel.addCard(code);
    }
    function removeCard(index) { memory.mModel.removeCard(index); }
    function getXForNewCard() { return memory.x; }
    function getYForNewCard() { return memory.y }
    function getXForCard() { return memory.x; }
    function getYForCard() { return memory.y; }
    function scaleForMovingCard() { return memory.scale; }
}
