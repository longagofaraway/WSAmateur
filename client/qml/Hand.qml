import QtQuick 2.12
import QtQml.Models 2.12
import QtGraphicalEffects 1.12

import "objectCreation.js" as CardCreator

ListView {
    id: handView

    property bool opponent
    property real length: getHandLength(count)
    property int dragIndex: -1

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

    function getAngle(index, middle) {
        return (index - middle) * 3 * (opponent ? -1 : 1);
    }
    function getHandLength(count) {
        if (!count)
            return 0;
        return (count - 1) * root.cardWidth * 2/3 + root.cardWidth;
    }
    function getFanOffset(index) {
        var radius = 1700;
        var pos = index * root.cardWidth * 2/3 + root.cardWidth / 2;
        var x = -(handView.length / 2 - pos);
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
                if (opponent)
                    return;

                var tempDragIndex = cardImgDelegate.visualIndex;
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
                }

                onExited: {
                    if (cardImgDelegate.state === "hovered") {
                        cardImgDelegate.state = "";
                    }
                }
            }

            Card {
                id: cardImgDelegate

                source: src

                property int visualIndex: 0
                property var cardCode: code

                anchors {
                    horizontalCenter: parent === cardDelegate ? parent.horizontalCenter : undefined
                    verticalCenter: parent === cardDelegate ? parent.verticalCenter : undefined
                }

                states: [
                    State {
                        name: "hovered"
                        PropertyChanges {
                            target: cardImgDelegate
                            anchors {
                                horizontalCenter: undefined
                                verticalCenter: undefined
                            }
                            scale: 1.5
                            anchors.verticalCenterOffset: -(getFanOffset(cardDelegate.visualIndex) + (root.cardHeight / 2 - 8) * 1.5)
                            z: 100
                        }
                        ParentChange {
                            target: cardImgDelegate
                            parent: handView.contentItem
                            rotation: parent.rotation
                        }
                        StateChangeScript {
                            name: "textFrame"
                            script: createTextFrame(cardDelegate, cardImgDelegate);
                        }
                    },
                    State {
                        name: "dragged"
                        PropertyChanges {
                            target: cardImgDelegate
                            anchors{
                                horizontalCenter: undefined
                                verticalCenter: undefined
                            }
                            z: 100
                            scale: 1
                        }
                        ParentChange {
                            target: cardImgDelegate
                            parent: handView.contentItem
                            rotation: parent.rotation
                        }
                    }
                ]

                transitions: [
                    Transition {
                        from: "*"
                        to: "hovered"
                        SequentialAnimation {
                            ParallelAnimation {
                                NumberAnimation {
                                    target: cardImgDelegate
                                    property: "scale"
                                    duration: 50
                                    easing.type: Easing.OutElastic
                                    easing.amplitude: 2.6
                                    easing.period: 2.0
                                }
                                NumberAnimation {
                                    target: cardImgDelegate
                                    property: "anchors.verticalCenterOffset"
                                    duration: 17
                                }
                                RotationAnimation {
                                    duration: 17
                                }
                            }
                            ScriptAction { scriptName: "textFrame" }
                        }
                    },
                    Transition {
                        from: "hovered"
                        to: ""
                        ParallelAnimation {
                            NumberAnimation {
                                target: cardImgDelegate
                                property: "scale"
                                duration: 150
                            }
                            NumberAnimation {
                                target: cardImgDelegate
                                property: "anchors.verticalCenterOffset"
                                duration: 150
                            }
                            RotationAnimation {
                                duration: 150
                            }
                            ScriptAction { script: destroyTextFrame(cardDelegate); }
                        }
                    },
                    Transition {
                        from: "hovered"
                        to: "dragged"
                        ParallelAnimation {
                            NumberAnimation {
                                target: cardImgDelegate
                                property: "scale"
                                duration: 150
                            }
                            NumberAnimation {
                                target: cardImgDelegate
                                property: "y"
                                duration: 50
                            }
                            ScriptAction { script: destroyTextFrame(cardDelegate); }
                        }
                    }
                ]

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    enabled: !opponent
                    drag.target: cardImgDelegate
                    drag.threshold: 0
                    drag.smoothed: true
                    drag.onActiveChanged: {
                        if (dragArea.drag.active) {
                            handView.dragIndex = cardDelegate.visualIndex;
                            cardImgDelegate.state = "dragged";
                        }
                    }
                    onReleased: {
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

                        aX.from = cardImgDelegate.x;
                        aX.to = cardDelegate.x;
                        aY.from = cardImgDelegate.y;
                        aY.to = cardDelegate.y;
                        dropAnim.start();
                    }
                }

                Drag.source: dragArea
                Drag.active: dragArea.drag.active

                Drag.hotSpot.x: root.cardWidth / 2
                Drag.hotSpot.y: root.cardHeight

                ParallelAnimation {
                    id: dropAnim
                    running: false

                    NumberAnimation {
                        id: aX
                        target: cardImgDelegate
                        property: "x"
                        duration: 150
                    }
                    NumberAnimation {
                        id: aY
                        target: cardImgDelegate
                        property: "y"
                        duration: 150
                    }
                    onFinished: {
                        cardImgDelegate.state = "";
                    }
                }

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
}
