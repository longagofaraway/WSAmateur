import QtQuick 2.12
import QtQuick.Window 2.12

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

ListView {
    id: levelZone

    property bool opponent
    property bool hidden: false
    property CardModel mModel: innerModel
    property real mMarginModifier: 0.2
    property real mMargin: root.cardHeight * mMarginModifier
    property real mScale: 0.8

    x: opponent ? (root.width * 0.85) : (root.width * 0.08)
    y: opponent ? (root.height * 0.15) : (root.height * 0.87 - getLevelHeight())
    width: root.cardWidth
    height: getLevelHeight()
    spacing: -root.cardHeight * (1 - mMarginModifier)
    interactive: false
    rotation: opponent ? 0 : 180

    model: mModel

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
    }

    delegate: Component {
        MouseArea {
            id: cardDelegate

            width: root.cardWidth
            height: root.cardHeight
            hoverEnabled: true
            scale: mScale
            rotation: 90
            z: model.index

            onEntered: {
                cardImgDelegate.state = "hovered";
            }
            onExited: {
                cardImgDelegate.state = "";
            }
            onClicked: {
                if (!model.glow)
                    return;
                model.selected = !model.selected;
                gGame.getPlayer().chooseCard(model.index, "level", levelZone.opponent);
            }

            Card {
                id: cardImgDelegate

                property CardTextFrame cardTextFrame: null

                mSource: model.code;
                anchors.centerIn: cardDelegate
                Component.onDestruction: destroyTextFrame(cardImgDelegate)

                states: State {
                    name: "hovered"
                    PropertyChanges {
                        target: cardImgDelegate
                        z: 100
                    }
                    ParentChange {
                        target: cardImgDelegate
                        parent: levelZone
                        scale: 0.9
                    }
                    StateChangeScript {
                        name: "textFrame"
                        script: createTextFrame(cardImgDelegate, model.index);
                    }
                }

                transitions: Transition {
                    from: "hovered"
                    to: ""
                    ScriptAction { script: destroyTextFrame(cardImgDelegate); }
                }

                CardGlow {
                    glow: model.glow
                    selected: model.selected
                }
            }
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
            let textFrame = this.object;
            if (!opponent) {
                textFrame.anchors.bottom = frameParent.bottom;
                textFrame.transformOrigin = Item.BottomLeft;
            } else {
                textFrame.transformOrigin = Item.TopLeft;
                textFrame.y = frameParent.height + textFrame.width;
                textFrame.leftOrientedCardReferences = true;
            }
            textFrame.rotation = opponent ? -90 : 90;
            textFrame.mModel = levelZone.mModel.textModel(index);
            textFrame.visible = true;
            frameParent.cardTextFrame = textFrame;
        }

        if (levelZone.mModel.textModel(index).rowCount() > 0)
            ObjectCreator.createAsync("CardTextFrame", frameParent, cb);
    }

    function destroyTextFrame(frameParent) {
        if (frameParent.cardTextFrame !== null) {
            frameParent.cardTextFrame.destroy();
            frameParent.cardTextFrame = null;
        }
    }

    function getLevelHeight()  {
        if (!levelZone.count)
            return 0;
        return (levelZone.count - 1) * mMargin + root.cardHeight;
    }

    function addCard(id, code, targetPos) { gGame.getPlayer(opponent).addCard(id, code, "level", targetPos); }
    function removeCard(index) { levelZone.mModel.removeCard(index); }
    function getXForNewCard() { return levelZone.x; }
    function getYForNewCard() {
        if (opponent)
            return levelZone.y + levelZone.count * mMargin;
        return root.height * 0.87 - levelZone.count * mMargin - root.cardHeight;
    }
    function getXForCard() { return levelZone.x; }
    function getYForCard(index) {
        if (opponent)
            return levelZone.y + index * mMargin;
        return root.height * 0.87 - index * mMargin - root.cardHeight;
    }
    function scaleForMovingCard() { return levelZone.mScale; }
}
