import QtQuick 2.0

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

ListView {
    id: zone

    property bool opponent
    property bool hidden: false
    property CardModel mModel: innerModel
    property real mMarginModifier: 0.3
    property real mMargin: root.cardWidth * mMarginModifier

    x: (opponent ? (root.width * 0.22) : (root.width * 0.78)) - (count ? contentWidth / 2 : root.cardWidth / 2)
    y: opponent ? (root.height * 0.3) : (root.height * 0.6)
    width: contentWidth
    height: root.cardHeight
    spacing: -root.cardWidth * (1 - mMarginModifier)
    orientation: ListView.Horizontal
    interactive: false
    z: 50

    model: mModel

    delegate: Component {
        MouseArea {
            id: cardDelegate

            width: root.cardWidth
            height: root.cardHeight
            hoverEnabled: true

            onEntered: {
                cardImgDelegate.state = "hovered";
                mUnhoverAnim.stop();
                mHoverAnim.start();
            }
            onExited: {
                cardImgDelegate.state = "";
                mHoverAnim.stop();
                mUnhoverAnim.start();
            }

            Card {
                id: cardImgDelegate

                property CardTextFrame cardTextFrame: null

                mSource: model.code;
                anchors.centerIn: cardDelegate
                Component.onDestruction: destroyTextFrame(cardImgDelegate)

                SequentialAnimation {
                    id: mHoverAnim
                    PropertyAction { target: cardImgDelegate; property: "parent"; value: zone.contentItem }
                    PropertyAction { target: cardImgDelegate; property: "z"; value: 100 }
                    PropertyAction { target: cardImgDelegate; property: "scale"; value: 1.1 }
                    PropertyAction { target: cardImgDelegate; property: "x"; value: model.index * root.cardWidth * (1 - mMarginModifier) }
                    ScriptAction { script: createTextFrame(cardImgDelegate, model.index); }
                }
                SequentialAnimation {
                    id: mUnhoverAnim
                    PropertyAction { target: cardImgDelegate; property: "parent"; value: cardDelegate }
                    PropertyAction { target: cardImgDelegate; property: "z"; value: 0 }
                    PropertyAction { target: cardImgDelegate; property: "scale"; value: 1 }
                    PropertyAction { target: cardImgDelegate; property: "x"; value: 0 }
                    ScriptAction { script: destroyTextFrame(cardImgDelegate); }
                }
            }
        }
    }
    Behavior on x { NumberAnimation { duration: 200 } }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
    }

    function createTextFrame(frameParent, index) {
        const cb = function(status) {
            if (status !== Component.Ready)
                return;

            if (frameParent.state !== "hovered") {
                this.object.destroy();
                return;
            }
            if (frameParent.cardTextFrame !== null)
                frameParent.cardTextFrame.destroy();
            let textFrame = this.object;
            if (opponent)
                textFrame.anchors.left = frameParent.right;
            else
                textFrame.anchors.right = frameParent.left;

            textFrame.mModel = zone.mModel.textModel(index);
            textFrame.visible = true;
            frameParent.cardTextFrame = textFrame;
        }

        if (zone.mModel.textModel(index).rowCount() > 0)
            ObjectCreator.createAsync("CardTextFrame", frameParent, cb);
    }

    function destroyTextFrame(frameParent) {
        if (frameParent.cardTextFrame !== null) {
            frameParent.cardTextFrame.destroy();
            frameParent.cardTextFrame = null;
        }
    }

    function addCard(id, code) { gGame.getPlayer(opponent).addCard(id, code, "res"); }
    function removeCard(index) { zone.mModel.removeCard(index); }
    function getXForNewCard() { return zone.x + zone.count * mMargin; }
    function getYForNewCard() { return zone.y; }
    function getXForCard(index) { return zone.x + index * mMargin; }
    function getYForCard() { return zone.y; }
}
