import QtQuick 2.0

import wsamateur.cardModel 1.0

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

                property CardInfoFrame cardTextFrame: null

                mSource: model.code;
                anchors.centerIn: cardDelegate
                Component.onDestruction: destroyTextFrame(cardImgDelegate)

                SequentialAnimation {
                    id: mHoverAnim
                    PropertyAction { target: cardImgDelegate; property: "parent"; value: zone.contentItem }
                    PropertyAction { target: cardImgDelegate; property: "z"; value: 100 }
                    PropertyAction { target: cardImgDelegate; property: "scale"; value: 1.1 }
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
            }
        }
    }
    Behavior on x { NumberAnimation { duration: 200 } }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
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
                let cardOffset = frameParent.x * zone.scale;

                if (opponent) {
                    let cardWidthAndScaleOffset = frameParent.width * zone.scale * (frameParent.scale + 1) / 2;
                    textFrame.x = zone.x + cardOffset + cardWidthAndScaleOffset;
                    textFrame.y = cardMappedPoint.y;

                    let cardHeight = frameParent.height * zone.scale * frameParent.scale;
                    if (textFrame.height > cardHeight)
                        textFrame.y -= textFrame.height - cardHeight;
                } else {
                    textFrame.x = zone.x - textFrame.width + cardOffset;
                    textFrame.y = cardMappedPoint.y;
                }

                if (frameParent.state !== "hovered") {
                    textFrame.destroy();
                    return;
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

    function addCard(code) { zone.mModel.addCard(code); }
    function removeCard(index) { zone.mModel.removeCard(index); }
    function getXForNewCard() { return zone.x + zone.count * mMargin; }
    function getYForNewCard() { return zone.y; }
    function getXForCard(index) { return zone.x + index/*(index ? (index - 1) : 0)*/ * mMargin; }
    function getYForCard() { return zone.y; }
}
