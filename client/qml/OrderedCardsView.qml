import QtQuick 2.15
import QtQml.Models 2.15

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

Rectangle {
    id: cardsView

    property bool opponent
    property int mViewMode: Game.RevealMode
    property bool hidden: opponent && mViewMode === Game.LookMode
    property CardModel mModel: innerModel
    property bool mDragEnabled: false

    Connections {
        target: mModel
        function onCountChanged() {
            if (cardsView.mModel.count == 0)
                cardsView.opacity = 0;
            else
                cardsView.opacity = 1;
        }
    }

    x: opponent ? (root.width * 0.12) : (root.width * 0.89 - width)
    y: root.height / 2 + height * (opponent ? -1 : 0);
    z: 150
    width: Math.max(350, listView.x * 2 + listView.contentWidth)
    height: header.contentHeight + listView.height + 25
    color: "#F0564747"
    radius: 5
    border.width: 1
    border.color: "white"
    opacity: 0

    Behavior on opacity { NumberAnimation { duration: 50 } }

    Text {
        id: header
        anchors.horizontalCenter: parent.horizontalCenter
        text: mViewMode === Game.RevealMode ? "Revealing" : "Looking"
        font.family: "Futura Bk BT"
        font.pointSize: 20
        color: "white"
    }

    ListView {
        id: listView

        property real mMarginModifier: 0.3
        property real mMargin: root.cardWidth * mMarginModifier
        property bool mDragActive: false

        height: root.cardHeight
        width: contentWidth
        x: 15
        anchors.top: header.bottom
        anchors.topMargin: 10
        spacing: -root.cardWidth * (1 - mMarginModifier)
        orientation: ListView.Horizontal
        interactive: false
        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 200 }
        }

        model: DelegateModel {
            id: visualModel

            model: mModel
            delegate: DropArea {
                id: delegateRoot

                property int visualIndex: DelegateModel.itemsIndex

                width: root.cardWidth
                height: root.cardHeight
                z: visualIndex

                onEntered: function(drag) {
                    visualModel.items.move((drag.source as Card).visualIndex, cardImg.visualIndex)
                }

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    hoverEnabled: true
                }

                Card {
                    id: cardImg

                    property int visualIndex: delegateRoot.visualIndex
                    property CardTextFrame cardTextFrame: null

                    mSource: {
                        if (model.code)
                            return model.code;
                        return "cardback";
                    }

                    DragHandler {
                        id: dragHandler
                        dragThreshold: 0
                        enabled: cardsView.mDragEnabled
                    }

                    Drag.active: dragHandler.active
                    Drag.source: cardImg

                    states: [
                        State {
                            name: "hovered"
                            when: mouseArea.containsMouse && !dragHandler.active && !listView.mDragActive
                            ParentChange {
                                target: cardImg
                                parent: cardsView
                                scale: 1.1
                            }
                            StateChangeScript {
                                name: "textFrame"
                                script: createTextFrame(cardImg, cardsView.mModel.textModel(model.index));
                            }
                        },
                        State {
                            name: "dragged"
                            when: dragHandler.active
                            ParentChange {
                                target: cardImg
                                parent: cardsView
                            }
                            PropertyChanges {
                                target: listView
                                mDragActive: true
                            }
                        }
                    ]

                    transitions: Transition {
                        from: "hovered"
                        ScriptAction { script: destroyTextFrame(cardImg); }
                    }
                }
            }
        }
    }

    SequentialAnimation {
        id: clearAnim
        NumberAnimation {
            target: cardsView
            property: "opacity"
            to: 0
            duration: 50
        }
        ScriptAction { script: cardsView.mModel.clear(); }
    }

    function destroyTextFrame(frameParent) {
        if (frameParent.cardTextFrame !== null) {
            frameParent.cardTextFrame.destroy();
            frameParent.cardTextFrame = null;
        }
    }

    function createTextFrame(frameParent, model) {
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
            if (!opponent)
                textFrame.anchors.right = frameParent.left;
            else
                textFrame.anchors.left = frameParent.right;

            textFrame.mModel = model;
            textFrame.visible = true;
            frameParent.cardTextFrame = textFrame;
        }

        ObjectCreator.createAsync("CardTextFrame", frameParent, cb);
    }

    function clear() { clearAnim.start(); }

    function addCard(code) { cardsView.mModel.addCard(code); }
    function removeCard(index) { cardsView.mModel.removeCard(index); }
    function getXForNewCard() { return cardsView.x + listView.x + listView.count * listView.mMargin; }
    function getYForNewCard() { return cardsView.y + listView.y; }
    function getXForCard(pos) { return cardsView.x + listView.x + pos * listView.mMargin; }
    function getYForCard() { return cardsView.y + listView.y; }
}
