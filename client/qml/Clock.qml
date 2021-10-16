import QtQuick 2.15

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

ListView {
    id: clockView

    property bool opponent
    property bool hidden: false
    property CardModel mModel: innerModel
    property real mMarginModifier: 0.3
    property real mMargin: root.cardWidth * mMarginModifier
    property real mScale: 0.7

    x: opponent ? (root.width * 0.7) : (root.width * 0.08)
    y: opponent ? (-root.height * 0.01) : (root.height * 0.85)
    width: contentWidth
    height: root.cardHeight
    spacing: -root.cardWidth * (1 - mMarginModifier)
    orientation: ListView.Horizontal
    interactive: false
    z: 55

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
    }

    model: mModel

    delegate: Component {
        MouseArea {
            id: cardDelegate

            width: root.cardWidth
            height: root.cardHeight
            hoverEnabled: true
            scale: mScale
            z: model.index

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
            onClicked: {
                if (clockView.state === "levelup" && model.index < 7) {
                    clockView.state = "";
                    model.selected = !model.selected;
                    glow7Cards(false);
                    gGame.getPlayer(opponent).cardSelectedForLevelUp(model.index);
                } else {
                    if (!model.glow)
                        return;
                    model.selected = !model.selected;
                    gGame.getPlayer().chooseCard(model.index, "clock", clockView.opponent);
                }
            }

            Card {
                id: cardImgDelegate

                property CardTextFrame cardTextFrame: null

                mSource: model.code;
                anchors.centerIn: cardDelegate
                Component.onDestruction: destroyTextFrame(cardImgDelegate)

                SequentialAnimation {
                    id: mHoverAnim
                    PropertyAction { target: cardImgDelegate; property: "parent"; value: clockView.contentItem }
                    PropertyAction { target: cardImgDelegate; property: "z"; value: 100 }
                    PropertyAction { target: cardImgDelegate; property: "scale"; value: 0.8 }
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

                CardGlow {
                    glow: model.glow
                    selected: model.selected
                }
            }
        }
    }

    function levelUp() {
        glow7Cards(true);
        clockView.state = "levelup";
    }

    function glow7Cards(glow) {
        for (let i = 0; i < Math.min(clockView.mModel.rowCount(), 7); i++) {
            let index = clockView.mModel.index(i, 0);
            clockView.mModel.setData(index, glow, CardModel.GlowRole);
        }
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
            var textFrame = this.object;

            if (!opponent)
                textFrame.anchors.left = frameParent.right;
            else
                textFrame.anchors.right = frameParent.left;
            textFrame.mModel = clockView.mModel.textModel(index);
            textFrame.visible = true;
            frameParent.cardTextFrame = textFrame;
        }

        if (clockView.mModel.textModel(index).rowCount() > 0)
            ObjectCreator.createAsync("CardTextFrame", frameParent, cb);
    }

    function destroyTextFrame(frameParent) {
        if (frameParent.cardTextFrame !== null) {
            frameParent.cardTextFrame.destroy();
            frameParent.cardTextFrame = null;
        }
    }

    function addCard(id, code, targetPos) { gGame.getPlayer(opponent).addCard(id, code, "clock", targetPos); }
    function removeCard(index) { clockView.mModel.removeCard(index); }
    function getXForNewCard() { return clockView.x + clockView.count * mMargin; }
    function getYForNewCard() { return clockView.y; }
    function getXForCard(pos) { return clockView.x + pos * mMargin; }
    function getYForCard() { return clockView.y; }
    function scaleForMovingCard() { return mScale; }
}
