import QtQuick 2.15
import QtQml.Models 2.15
import QtGraphicalEffects 1.15

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

ListView {
    id: handView

    property bool opponent
    property bool hidden: opponent
    property real length: getHandLength(count)
    property int dragIndex: -1
    property MulliganHeader mHeader: null
    property CardModel mModel: innerModel
    property var mLastDragPosition: null
    property int mLastDragCardId

    width: length
    height: root.cardHeight
    x: root.width / 2 - length / 2
    y: calculateY()
    orientation: ListView.Horizontal
    spacing: -root.cardWidth  / 3
    interactive: false
    z: 50

    function calculateY() {
        if (opponent) {
            var base = 0;
            var multp = 1;
        } else {
            base = root.height;
            multp = -1;
        }
        return base - root.cardHeight / 2 + 10 * multp;
    }

    // states don't work with reparenting in this case
    SequentialAnimation {
        id: mMulliganAnim
        ScriptAction { script: gGame.startUiAction(); }
        PropertyAction { target: handView; property: "parent"; value: root }
        ParallelAnimation {
            NumberAnimation {
                target: handView;
                property: "y";
                to: root.height / 2 - root.cardHeight / 2;
                easing.type: Easing.OutExpo;
                duration: 300
            }
            NumberAnimation {
                target: handView;
                property: "scale";
                to: 1.5;
                easing.type: Easing.OutExpo;
                duration: 300
            }
        }
        ScriptAction { script: gGame.uiActionComplete(); }
    }
    SequentialAnimation {
        id: mEndMulliganAnim
        ScriptAction { script: gGame.startUiAction(); }
        PropertyAction { target: handView; property: "parent"; value: gGame }
        ParallelAnimation {
            NumberAnimation {
                target: handView;
                property: "y";
                to: calculateY()
                easing.type: Easing.OutExpo;
                duration: 300
            }
            NumberAnimation {
                target: handView;
                property: "scale";
                to: 1;
                easing.type: Easing.OutExpo;
                duration: 300
            }
        }
        ScriptAction { script: gGame.uiActionComplete(); }
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

                mSource: {
                    if (model.code)
                        return model.code;
                    return "cardback";
                }

                property CardTextFrame cardTextFrame: null
                property int visualIndex: 0
                property var enlargeAnim: mEnlargeAnim
                property var shrinkAnim: mShrinkAnim
                property string cardType: opponent ? "" : model.type
                property bool cannotMove: model.cannotMove

                Component.onDestruction: destroyTextFrame(cardImgDelegate)
                anchors.centerIn: cardDelegate

                // state transitions aren't suitable for this task as we cannot stop transition animations
                ParallelAnimation {
                    id: mShrinkToDragAnim
                    ScriptAction { script: destroyTextFrame(cardImgDelegate); }
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
                    ScriptAction { script: destroyTextFrame(cardImgDelegate); }
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
                    ScriptAction { script: createTextFrame(cardImgDelegate, model.index); }
                }

                MouseArea {
                    id: dragArea

                    property bool dragActive: false

                    anchors.fill: parent
                    enabled: !opponent

                    onClicked: {
                        if (handView.state === "mulligan") {
                            model.glow = !model.glow;
                            gGame.changeCardCountForMulligan(model.glow);
                        } else if (handView.state === "clock") {
                            if (!model.glow)
                                return;
                            model.selected = !model.selected;
                            gGame.changeCardCountForClock(model.selected);
                            if (model.selected)
                                unglowUnselected();
                            else
                                glowAllCards(true);
                        } else if (handView.state === "discardTo7") {
                            if (!model.glow)
                                return;
                            gGame.getPlayer(opponent).sendDiscardCard(model.index);
                            glowAllCards(false);
                            handView.state = "";
                        } else if (handView.state !== "playTiming") {
                            if (!model.glow)
                                return;
                            model.selected = !model.selected;
                            gGame.getPlayer().chooseCard(model.index, "hand", handView.opponent);
                        }
                    }

                    // we need manual drag because I couldn't adjust center of the card to the cursor
                    onPositionChanged: {
                        if (handView.state !== "playTiming" || !model.glow)
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
                        if (point.y < -170 && !model.selected
                                && (cardImgDelegate.cardType === "Climax" || cardImgDelegate.cardType === "Event"
                                    || gGame.isCounterStep())) {
                            if (cardImgDelegate.cardType === "Climax")
                                cardImgDelegate.rotation = -90;
                            setSelected(model.index, true);
                        }
                        else if (point.y >= -170 && model.selected
                                 && (cardImgDelegate.cardType === "Climax" || cardImgDelegate.cardType === "Event"
                                     || gGame.isCounterStep())) {
                            if (cardImgDelegate.cardType === "Climax")
                                cardImgDelegate.rotation = 0;
                            setSelected(model.index, false);
                        }

                        cardImgDelegate.x = point.x - root.cardWidth / 2;
                        cardImgDelegate.y = point.y - root.cardHeight / 2;
                    }

                    onReleased: {
                        if (!dragActive)
                            return;
                        handView.dragIndex = -1;
                        if (root.stageDropTarget !== undefined) {
                            gGame.getPlayer(opponent).cardPlayed(model.index, root.stageDropTarget.mIndex);
                            if (root.stageDropTarget.mStageCard !== null)
                                root.stageDropTarget.sendToWr();
                            root.stageDropTarget.setCard(model.cardId, model.code);
                            dragActive = false;
                            handView.mModel.removeCard(model.index);
                            root.stageDropTarget = undefined;
                            return;
                        }
                        dragActive = false;
                        if (model.selected) {
                            if (cardImgDelegate.cardType === "Climax") {
                                handView.mLastDragPosition = cardImgDelegate.mapToItem(gGame, 0, 0);
                                handView.mLastDragCardId = model.cardId;
                                gGame.player.cardPlayed(model.index, 0);
                                return;
                            } else if (gGame.isCounterStep()) {
                                handView.mLastDragPosition = cardImgDelegate.mapToItem(gGame, 0, 0);
                                gGame.player.sendPlayCounter(model.index);
                                return;
                            } else if (cardImgDelegate.cardType === "Event") {
                                handView.mLastDragPosition = cardImgDelegate.mapToItem(gGame, 0, 0);
                                gGame.player.cardPlayed(model.index, 0);
                                return;
                            }
                        }
                        cardImgDelegate.state = "";
                        mDropAnim.start();
                    }
                }

                Drag.source: cardImgDelegate
                Drag.active: dragArea.dragActive

                Drag.hotSpot.x: root.cardWidth / 2
                Drag.hotSpot.y: root.cardHeight

                RectangularGlow {
                    id: cardGlow
                    anchors.fill: cardImgDelegate
                    z: -1
                    color: model.selected ? "#FCDE01" : "#2BFDFF"
                    cornerRadius: 0
                    glowRadius: 10
                    visible: model.glow
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
                Behavior on rotation { NumberAnimation { duration: 100 } }
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

    function createTextFrame(cardImgDelegate, index) {
        const cb = function(status) {
            if (status !== Component.Ready)
                return;

            if (cardImgDelegate.state !== "hovered") {
                this.object.destroy();
                return;
            }
            if (cardImgDelegate.cardTextFrame !== null)
                cardImgDelegate.cardTextFrame.destroy();
            let textFrame = this.object;
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
            textFrame.mModel = handView.mModel.textModel(index);
            textFrame.visible = true;
            textFrame.z = -2;
            cardImgDelegate.cardTextFrame = textFrame;
            return;
        }

        if (handView.mModel.textModel(index).rowCount() > 0)
            ObjectCreator.createAsync("CardTextFrame", cardImgDelegate, cb);
    }

    function destroyTextFrame(cardImgDelegate) {
        if (cardImgDelegate.cardTextFrame !== null) {
            cardImgDelegate.cardTextFrame.destroy();
            cardImgDelegate.cardTextFrame = null;
        }
    }

    function mulligan() {
        handView.state = "mulligan";
        mMulliganAnim.start();
    }

    function endMulligan() {
        handView.state = "";
        mEndMulliganAnim.start();
    }

    function clockPhase() {
        handView.state = "clock";
        glowAllCards(true);
    }

    function endClockPhase() {
        handView.state = "";
        glowAllCards(false);
    }

    function discardCard() {
        handView.state = "discardTo7";
        glowAllCards(true);
    }

    function glowAllCards(glow) {
        for (let i = 0; i < handView.mModel.rowCount(); i++) {
            const index = handView.mModel.index(i, 0);
            handView.mModel.setData(index, glow, CardModel.GlowRole);
        }
    }

    function setSelected(index, selected) {
        const modelIndex = handView.mModel.index(index, 0);
        handView.mModel.setData(modelIndex, selected, CardModel.SelectedRole);
    }

    function unglowUnselected() {
        for (let i = 0; i < handView.mModel.rowCount(); i++) {
            const index = handView.mModel.index(i, 0);
            const selected = handView.mModel.data(index, CardModel.SelectedRole);
            if (selected)
                continue;
            handView.mModel.setData(index, false, CardModel.GlowRole);
        }
    }

    function addCard(id, code) {
        if (hidden) gGame.getPlayer(opponent).addCard(0, "", "hand");
        else gGame.getPlayer(opponent).addCard(id, code, "hand");
    }
    function removeCard(index) { handView.mModel.removeCard(index); }

    function getVisualIndexFromModelIndex(modelId) {
        for (let i = 0; i < handDelegate.count; i++) {
            if (handDelegate.items.get(i).model.index === modelId)
                return i;
        }
    }

    function getXForNewCard() { return handView.x + handDelegate.count * root.cardWidth * 2/3; }
    function getYForNewCard() { return handView.y + getFanOffset(handDelegate.count); }
    function getXForCard(modelId) {
        if (handView.mLastDragPosition !== null) {
            const index = handView.mModel.index(modelId, 0);
            const cardId = handView.mModel.data(index, CardModel.CardIdRole);
            if (cardId === handView.mLastDragCardId)
                return handView.mLastDragPosition.x;
        }

        return handView.x + getVisualIndexFromModelIndex(modelId) * root.cardWidth * 2/3;
    }
    function getYForCard(modelId) {
        if (handView.mLastDragPosition !== null) {
            const index = handView.mModel.index(modelId, 0);
            const cardId = handView.mModel.data(index, CardModel.CardIdRole);
            if (cardId === handView.mLastDragCardId) {
                const y = handView.mLastDragPosition.y;
                handView.mLastDragPosition = null;
                return y;
            }
        }

        return handView.y + getFanOffset(getVisualIndexFromModelIndex(modelId));
    }
}
