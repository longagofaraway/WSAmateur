import QtQuick 2.15

import wsamateur.cardModel 1.0

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
                    ScriptAction { script: createTextFrame(cardImgDelegate); }
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

    function createTextFrame(frameParent) {
        let comp = Qt.createComponent("CardTextFrame.qml");
        let incubator = comp.incubateObject(root, { visible: false }, Qt.Asynchronous);
        let createdCallback = function(status) {
            if (status === Component.Ready) {
                if (frameParent.state !== "hovered") {
                    incubator.object.destroy();
                    return;
                }
                if (frameParent.cardTextFrame !== null)
                    frameParent.cardTextFrame.destroy();
                var textFrame = incubator.object;

                let cardMappedPoint = root.mapFromItem(frameParent, frameParent.x, frameParent.y);
                let cardOffset = frameParent.x * clockView.scale;

                if (!opponent) {
                    let cardWidthAndScaleOffset = frameParent.width * clockView.scale * (frameParent.scale + 1) / 2;
                    textFrame.x = clockView.x + cardOffset + cardWidthAndScaleOffset;
                    textFrame.y = cardMappedPoint.y;

                    let cardHeight = frameParent.height * clockView.scale * frameParent.scale;
                    if (textFrame.height > cardHeight)
                        textFrame.y -= textFrame.height - cardHeight;
                } else {
                    textFrame.x = clockView.x - textFrame.width + cardOffset;
                    textFrame.y = cardMappedPoint.y;
                }

                textFrame.visible = true;
                frameParent.cardTextFrame = textFrame;
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

    function addCard(code) { clockView.mModel.addCard(code); }
    function removeCard(index) { clockView.mModel.removeCard(index); }
    function getXForNewCard() { return clockView.x + clockView.count * mMargin; }
    function getYForNewCard() { return clockView.y; }
    function getXForCard(pos) { return clockView.x + pos * mMargin; }
    function getYForCard() { return clockView.y; }
    function scaleForMovingCard() { return mScale; }
}
