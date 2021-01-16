import QtQuick 2.12

import wsamateur.cardModel 1.0

ListView {
    id: clockView

    signal cardSelected(int index)
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

    transitions: Transition {
        SequentialAnimation {
            ScriptAction { script: gGame.startUiAction(); }
            ParallelAnimation {
                NumberAnimation { property: "x"; easing.type: Easing.OutExpo; duration: 300 }
                NumberAnimation { property: "y"; easing.type: Easing.OutExpo; duration: 300 }
                NumberAnimation { property: "scale"; easing.type: Easing.OutExpo; duration: 300 }
            }
            ScriptAction { script: gGame.uiActionComplete(); }
        }
    }

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
            }
            onExited: {
                cardImgDelegate.state = "";
            }
            onClicked: {
                if (clockView.state === "levelup" && model.index < 7) {
                    clockView.state = "";
                    model.selected = !model.selected;
                    glow7Cards(false);
                    cardSelected(model.index);
                }
            }

            Card {
                id: cardImgDelegate

                property CardInfoFrame cardTextFrame: null

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
                        parent: clockView.contentItem
                        scale: 0.8
                    }
                    StateChangeScript {
                        name: "textFrame"
                        script: createTextFrame(cardImgDelegate);
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
        let comp = Qt.createComponent("CardInfoFrame.qml");
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
    function getXForNewCard() { return clockView.x + clockView.count * mMargin; }
    function getYForNewCard() { return clockView.y; }
    function getXForCard(pos) { return clockView.x + pos * mMargin; }
    function getYForCard() { return clockView.y; }
    function scaleForMovingCard() { return mScale; }
}
