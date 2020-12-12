import QtQuick 2.12
import QtQml.Models 2.12
import QtGraphicalEffects 1.12

import wsamateur.cardModel 1.0

import "objectCreation.js" as CardCreator

ListView {
    id: handView

    signal cardSelected(bool selected)
    signal moveFinished()
    property bool opponent
    property real length: getHandLength(count)
    property int dragIndex: -1
    property bool selectable: false
    property MulliganHeader mHeader: null
    property string moveDestination: "wr"
    property CardModel mModel: innerModel

    width: length
    height: root.cardHeight
    x: root.width / 2 - length / 2
    y: {
        if (opponent) {
            var base = 0;
            var multp = 1;
        } else {
            base = root.height;
            multp = -1;
        }
        return base - root.cardHeight / 2 + 10 * multp;
    }
    orientation: ListView.Horizontal
    spacing: -root.cardWidth  / 3
    interactive: false
    z: 1

    states: State {
        name: "mulligan"
        ParentChange {
            target: handView
            parent: root
        }
        PropertyChanges {
            target: handView
            scale: 1.5
            y: root.height / 2 - root.cardHeight / 2
            z: 100
            selectable: true
        }
    }

    transitions: Transition {
        SequentialAnimation {
            NumberAnimation { property: "y"; easing.type: Easing.OutExpo; duration: 300 }
            NumberAnimation { property: "scale"; easing.type: Easing.OutExpo; duration: 300 }
            ScriptAction { script: gGame.actionComplete() }
        }
    }

    function getAngle(index, middle) {
        return (index - middle) * 3 * (opponent ? -1 : 1);
    }
    function getHandLength(count) {
        if (!count)
            return 0;
        return (count - 1) * root.cardWidth * 2/3 + root.cardWidth;
    }
    function getFanOffset(index) {
        let radius = 1700;
        let pos = index * root.cardWidth * 2/3 + root.cardWidth / 2;
        let x = -(handView.length / 2 - pos);
        return (Math.sqrt(radius**2 - x**2) - radius) * (opponent ? 1 : -1);
    }

    model: DelegateModel {
        id: handDelegate

        property real halfIndex: (count ? (count - 1) : count) / 2
        model: mModel

        delegate: DropArea {
            id: cardDelegate

            property string mCode: code
            property bool mGlow: glow

            property CardInfoFrame cardTextFrame: null
            property int visualIndex: DelegateModel.itemsIndex

            width: root.cardWidth
            height: root.cardHeight
            z: DelegateModel.itemsIndex

            Binding { target: cardImgDelegate; property: "visualIndex"; value: visualIndex }

            onEntered: {
                if (opponent || handView.dragIndex == -1)
                    return;

                let tempDragIndex = cardImgDelegate.visualIndex;
                handDelegate.items.move(
                        handView.dragIndex,
                        cardImgDelegate.visualIndex);
                handView.dragIndex = tempDragIndex;
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                enabled: !opponent
                onEntered: {
                    cardImgDelegate.state = "hovered";
                    cardImgDelegate.shrinkAnim.stop();
                    cardImgDelegate.enlargeAnim.start();
                }

                onExited: {
                    if (cardImgDelegate.state === "hovered") {
                        cardImgDelegate.state = "";
                        cardImgDelegate.enlargeAnim.stop();
                        cardImgDelegate.shrinkAnim.start();
                    }
                }
            }

            Card {
                id: cardImgDelegate

                source: "image://imgprov/" + cardDelegate.mCode

                property int visualIndex: 0
                property var cardCode: code
                property var enlargeAnim: mEnlargeAnim
                property var shrinkAnim: mShrinkAnim

                anchors.centerIn: cardDelegate

                // state transitions aren't suitable for this task as we cannot stop transition animations
                ParallelAnimation {
                    id: mShrinkToDragAnim
                    ScriptAction { script: destroyTextFrame(cardDelegate); }
                    NumberAnimation { target: cardImgDelegate; property: "scale"; to: 1 }
                }
                SequentialAnimation {
                    id: mDropAnim
                    ParallelAnimation {
                        NumberAnimation { target: cardImgDelegate; property: "x"; duration: 150; to: cardDelegate.x }
                        NumberAnimation { target: cardImgDelegate; property: "y"; duration: 150; to: cardDelegate.y }
                    }
                    PropertyAction { target: cardImgDelegate; property: "parent"; value: cardDelegate }
                    PropertyAction { target: cardImgDelegate; property: "x"; value: 0 }
                    PropertyAction { target: cardImgDelegate; property: "y"; value: 0 }
                    PropertyAction { target: cardImgDelegate; property: "rotation"; value: 0 }
                }

                SequentialAnimation {
                    id: mShrinkAnim
                    ScriptAction { script: destroyTextFrame(cardDelegate); }
                    ParallelAnimation {
                        NumberAnimation {
                            target: cardImgDelegate;
                            property: "y";
                            to:  {
                                if (handView.state === "mulligan")
                                    return getFanOffset(visualIndex);
                                return 0;
                            }
                            duration: 150;
                        }
                        NumberAnimation { target: cardImgDelegate; property: "scale"; to: 1; duration: 150 }
                        NumberAnimation { target: cardImgDelegate; property: "rotation"; to: cardDelegate.rotation; duration: 150 }
                    }
                    PropertyAction { target: cardImgDelegate; property: "parent"; value: cardDelegate }
                    PropertyAction { target: cardImgDelegate; property: "x"; value: 0 }
                    PropertyAction { target: cardImgDelegate; property: "y"; value: 0 }
                    PropertyAction { target: cardImgDelegate; property: "rotation"; value: 0 }
                }

                SequentialAnimation {
                    id: mEnlargeAnim
                    PropertyAction { target: cardImgDelegate; property: "parent"; value: handView }
                    PropertyAction { target: cardImgDelegate; property: "x"; value: visualIndex * root.cardWidth * 2/3 }
                    ParallelAnimation {
                        NumberAnimation {
                            target: cardImgDelegate
                            property: "scale"
                            to: 1.5
                            duration: 100
                            easing.type: Easing.OutElastic
                            easing.amplitude: 2.6
                            easing.period: 2.0
                        }
                        NumberAnimation {
                            target: cardImgDelegate;
                            property: "y";
                            to: {
                                if (handView.state === "mulligan")
                                    return 0;
                                return -(root.cardHeight / 2 - 8) * 1.5;
                            }
                            duration: 35
                        }
                        NumberAnimation { target: cardImgDelegate; property: "rotation"; to: 0; duration: 35 }
                    }
                    ScriptAction { script: createTextFrame(cardDelegate, cardImgDelegate); }
                }

                MouseArea {
                    id: dragArea

                    property bool dragActive: false

                    anchors.fill: parent
                    enabled: !opponent

                    onClicked: {
                        if (handView.state === "mulligan") {
                            model.glow = !model.glow;
                            cardSelected(model.glow);
                        }
                    }

                    // we need manual drag because I couldn't adjust center of the card to the cursor
                    onPositionChanged: {
                        if (handView.state === "mulligan")
                            return;

                        if (!dragActive) {
                            handView.dragIndex = cardDelegate.visualIndex;
                            dragActive = true;
                            cardImgDelegate.state = "dragged";
                            mShrinkAnim.stop();
                            mEnlargeAnim.stop();
                            mShrinkToDragAnim.start();
                        }
                        let point = cardImgDelegate.mapToItem(handView, mouse.x, mouse.y);
                        cardImgDelegate.x = point.x - root.cardWidth / 2;
                        cardImgDelegate.y = point.y - root.cardHeight / 2;
                    }

                    onReleased: {
                        if (!dragActive)
                            return;
                        dragActive = false;
                        if (stageDropTarget !== undefined) {
                            for (var i = 0; i < handDelegate.model.count; i++) {
                                if (handDelegate.model.get(i).code === cardImgDelegate.cardCode) {
                                    //create
                                    stageDropTarget.contentCard = CardCreator.createCard(stageDropTarget, "Card.qml");
                                    stageDropTarget.contentCard.source = cardImgDelegate.source;
                                    //delete
                                    handDelegate.model.remove(i);
                                    return;
                                }
                            }
                        }
                        cardImgDelegate.state = "";
                        mDropAnim.start();
                    }
                }

                Drag.source: dragArea
                Drag.active: dragArea.dragActive

                Drag.hotSpot.x: root.cardWidth / 2
                Drag.hotSpot.y: root.cardHeight

                RectangularGlow {
                    id: cardGlow
                    anchors.fill: cardImgDelegate
                    z: -1
                    color: "#2BFDFF"
                    cornerRadius: 0
                    glowRadius: 10
                    visible: cardDelegate.mGlow
                }
                SequentialAnimation {
                    running: cardGlow.visible
                    loops: 1000
                    NumberAnimation {
                        target: cardGlow
                        property: "spread"
                        from: 0
                        to: 0.1
                        duration: 2000
                    }
                    NumberAnimation {
                        target: cardGlow
                        property: "spread"
                        from: 0.1
                        to: 0
                        duration: 2000
                    }
                }
            }

            rotation: getAngle(visualIndex, handDelegate.halfIndex) + (opponent? 180 : 0)

            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: getFanOffset(visualIndex)

            Behavior on anchors.verticalCenterOffset {
                NumberAnimation { duration: 200 }
            }
        }
    }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
    }

    Behavior on x {
        NumberAnimation { duration: 200 }
    }

    function createTextFrame(cardDelegate, cardImgDelegate) {
        let comp = Qt.createComponent("CardInfoFrame.qml");
        let incubator = comp.incubateObject(cardImgDelegate, { visible: false }, Qt.Asynchronous);
        let createdCallback = function(status) {
            if (status === Component.Ready) {
                if (cardImgDelegate.state !== "hovered") {
                    incubator.object.destroy();
                    return;
                }
                if (cardDelegate.cardTextFrame !== null)
                    cardDelegate.cardTextFrame.destroy();
                var textFrame = incubator.object;
                textFrame.scale = 0.66;
                if (cardImgDelegate.visualIndex > handDelegate.halfIndex) {
                    textFrame.transformOrigin = Item.TopRight;
                    textFrame.anchors.right = cardImgDelegate.left;
                } else {
                    textFrame.transformOrigin = Item.TopLeft;
                    textFrame.anchors.left = cardImgDelegate.right;
                }
                textFrame.anchors.top = cardImgDelegate.top;
                if (textFrame.height * 0.66 > cardImgDelegate.height)
                    textFrame.anchors.topMargin = cardImgDelegate.height - textFrame.height * 0.66;
                textFrame.visible = true;
                cardDelegate.cardTextFrame = textFrame;
            }
        }
        if (incubator.status !== Component.Ready) {
            incubator.onStatusChanged = createdCallback;
        } else {
            createdCallback(Component.Ready);
        }
    }

    function destroyTextFrame(cardDelegate) {
        if (cardDelegate.cardTextFrame !== null) {
            cardDelegate.cardTextFrame.destroy();
            cardDelegate.cardTextFrame = null;
        }
    }

    function addCard(code) { handView.mModel.addCard(code); }

    function getXForNewCard() { return handView.x + handDelegate.count * root.cardWidth * 2/3; }
    function getXForCard(modelId) {
        let visualIndex = 0;
        for (var i = 0; i < handDelegate.count; i++) {
            let item = handDelegate.items.get(i);
            if (item.model.index === modelId) {
                visualIndex = i;
                break;
            }
        }

        return handView.x + visualIndex * root.cardWidth * 2/3;
    }
    function getYForNewCard() { return handView.y; }
    function getYForCard() { return handView.y; }

    MainButton {
        x: 500
        y: -100
        mText: "LOL"
        onClicked: {
            console.log(handDelegate.items.get(0).model);
            console.log(handDelegate.items.get(0).index);
            console.log(handDelegate.items.get(0).model.index);
            console.log(Object.keys(handDelegate.items.get(0).model));
            //handDelegate.model.removeCard(1);
        }
    }
}
