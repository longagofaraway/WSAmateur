import QtQuick 2.12
import QtQml.Models 2.12
import QtGraphicalEffects 1.12

import "objectCreation.js" as CardCreator

ListView {
    id: handView

    property bool opponent
    property real length: getHandLength(count)
    property int dragIndex: -1
    property bool mulligan: false
    property bool selectable: false
    property MulliganHeader mHeader: null

    property var mmodel: ListModel {
        ListElement { code: "IMC/W43-127"; glow: false }
        ListElement { code: "IMC/W43-111"; glow: false }
        ListElement { code: "IMC/W43-009"; glow: false }
        ListElement { code: "IMC/W43-046"; glow: false }
        ListElement { code: "IMC/W43-091"; glow: false }
    }

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
        model: innerModel

        delegate: DropArea {
            id: cardDelegate

            width: root.cardWidth
            height: root.cardHeight
            z: DelegateModel.itemsIndex

            property CardInfoFrame cardTextFrame: null
            property int visualIndex: DelegateModel.itemsIndex
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

                source: "image://imgprov/" + code

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
                                if (handView.mulligan)
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
                                if (handView.mulligan)
                                    return 0;//return getFanOffset(visualIndex);
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
                        //Object.keys(handDelegate.items.get(cardImgDelegate.visualIndex).model.index).forEach((prop)=> console.log(prop));

                        let ind = handDelegate.items.get(cardImgDelegate.visualIndex).model.index;
                        handDelegate.model[ind].glow = !handDelegate.model[ind].glow;
                        console.log(handDelegate.model[ind].glow);
                        //console.log(handDelegate.items.get(cardImgDelegate.visualIndex).model.index);
                        //print(handDelegate.model);
                    }

                    // we need manual drag because I couldn't adjust center of the card to the cursor
                    onPositionChanged: {
                        if (handView.mulligan)
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
                    color: "#55ff55"
                    cornerRadius: 0
                    glowRadius: 10
                    visible: glow
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

    function startMulligan() {
        handView.mulligan = true;
        handView.state = "mulligan";
        colorOverlay.color = "#D0000000";
        blurEffect.opacity = 1;

        var comp = Qt.createComponent("MulliganHeader.qml");
        mHeader = comp.createObject(root);
    }
}
