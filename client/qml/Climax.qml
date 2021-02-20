import QtQuick 2.0

import wsamateur 1.0

ListView {
    id: climaxZone

    property bool opponent
    property bool hidden: false
    property int mHandIndex
    property CardModel mModel: innerModel

    x: opponent ? (root.width * 0.8 - root.cardWidth) : (root.width * 0.2)
    y: opponent ? (root.height * 0.47 - root.cardHeight) : (root.height * 0.53)
    width: root.cardWidth
    height: root.cardHeight
    interactive: false
    rotation: -90

    model: mModel

    delegate: Component {
        MouseArea {
            id: cardDelegate

            property CardTextFrame cardTextFrame: null

            width: root.cardWidth
            height: root.cardHeight
            hoverEnabled: true
            //Component.onDestruction: destroyTextFrame(cardDelegate)

            onEntered: {
                //createTextFrame(cardDelegate);
            }
            onExited: {
                //destroyTextFrame(cardDelegate);
            }

            Card {
                id: cardImgDelegate

                mSource: model.code;
                anchors.centerIn: cardDelegate
            }
        }
    }

    function createTextFrame(frameParent) {
        let comp = Qt.createComponent("CardTextFrame.qml");
        let incubator = comp.incubateObject(root, { visible: false, z: 100 }, Qt.Asynchronous);
        let createdCallback = function(status) {
            if (status === Component.Ready) {
                if (!frameParent.containsMouse) {
                    incubator.object.destroy();
                    return;
                }
                if (frameParent.cardTextFrame !== null)
                    frameParent.cardTextFrame.destroy();
                let cardInfo = incubator.object;
                let offset = (root.cardHeight - root.cardWidth) / 2;
                if (!opponent) {
                    cardInfo.x = climaxZone.x + offset + root.cardWidth;
                    cardInfo.y = climaxZone.y + offset;
                } else {
                    cardInfo.x = climaxZone.x - cardInfo.width - offset;
                    cardInfo.y = climaxZone.y + offset;
                }

                cardInfo.visible = true;
                frameParent.cardTextFrame = cardInfo;
            }
        }
        if (incubator.status !== Component.Ready) {
            incubator.onStatusChanged = createdCallback;
        } else {
            createdCallback(Component.Ready);
        }
    }

    function destroyTextFrame(frameParent) {
        if (frameParent.cardTextFrame !== null) {
            frameParent.cardTextFrame.destroy();
            frameParent.cardTextFrame = null;
        }
    }

    function addCard(code, handIndex) {
        climaxZone.mHandIndex = handIndex;
        climaxZone.mModel.addCard(code);
    }
    function removeCard(index) { climaxZone.mModel.removeCard(index); }
    function getXForNewCard() { return climaxZone.x; }
    function getYForNewCard() { return climaxZone.y; }
    function getXForCard() { return climaxZone.x; }
    function getYForCard() { return climaxZone.y; }
}
