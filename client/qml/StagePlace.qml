import QtQuick 2.12
import QtQml.Models 2.12
import QtGraphicalEffects 1.12

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

ListView {
    id: stagePlace

    property int mIndex
    property StageCard mStageCard: null
    property CardTextFrame mCardInfo: null
    property bool mTooltipsDisabled: false
    property var mStageRect
    property bool mTapped: false

    width: root.cardWidth; height: root.cardHeight
    y: {
        let yy = root.height * 0.03;
        if (mIndex > 2)
            yy = yy * 2 + root.cardHeight;
        if (stage.opponent)
            yy = -yy - root.cardHeight;
        return yy + root.height / 2;
    }
    interactive: false

    model: DelegateModel {
        model: stage.mModel

        groups: [
            DelegateModelGroup {
                includeByDefault: false
                name: "stageCard"
            }
        ]
        filterOnGroup: "stageCard"

        items.onChanged: {
            for (let i = 0; i < items.count; i++) {
                let item = items.get(i);
                if (mIndex === item.model.index) {
                    item.groups = "stageCard";
                    break;
                }
            }
        }

        delegate: Rectangle {
            id: stageRect

            width: root.cardWidth
            height: root.cardHeight
            color: "#30FFFFFF"

            Component.onCompleted: { mStageRect = stageRect; }

            CardGlow {
                glow: model.glow && mStageCard == null
            }
            DropArea {
                id: dropStage
                width: root.cardWidth * 2
                height: stagePlace.mIndex < 3 ? root.cardHeight * 3/2 : root.cardHeight
                x: -(width - root.cardWidth) / 2
                y: stagePlace.mIndex < 3 ? 0 : root.cardHeight / 2

                onEntered: {
                    if (stage.opponent)
                        return;
                    if (drag.source.cardType === "Climax"
                        || drag.source.cardType === "Event"
                        || drag.source.cardType === undefined
                        || gGame.isCounterStep())
                        return;
                    if (drag.source.index === mIndex)
                        return;
                    if (model.cannotMove)
                        return;

                    drag.source.hoveringIndex = stagePlace.mIndex;
                    root.stageDropTarget = stagePlace;
                    if (mStageCard !== null)
                        mStageCard.onCardEntered();
                    else
                        stageRect.color = "#80FFFFFF";
                }

                onExited: {
                    if (stage.opponent)
                        return;
                    if (drag.source.index === mIndex)
                        return;

                    if (drag.source.hoveringIndex === stagePlace.mIndex)
                        root.stageDropTarget = undefined;
                    if (mStageCard !== null)
                        mStageCard.onCardExited();
                    else
                        stageRect.color = "#30FFFFFF";
                }
            }

            MouseArea {
                id: stagePlaceMouseArea

                anchors.fill: parent
                hoverEnabled: true
                z: 2
                rotation: (model.state === "Rested") ? 90 : 0
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                drag.target: (stage.opponent || !stage.mDragEnabled || model.cannotMove) ? undefined : mStageCard;
                drag.onActiveChanged: {
                    if (active)
                        destroyCardInfo();
                }

                Binding { target: mStageCard; property: "dragActive"; value: stagePlaceMouseArea.drag.active }
                Binding { target: mStageCard; property: "glow"; value: model.glow }
                Binding { target: mStageCard; property: "selected"; value: model.selected }
                Binding { target: mStageCard; property: "highlightedByAbility"; value: model.highlightedByAbility }
                Binding { target: mStageCard; property: "power"; value: model.power }
                Binding { target: mStageCard; property: "soul"; value: model.soul }
                Binding { target: mStageCard; property: "level"; value: model.level }
                Binding { target: mStageCard; property: "cardState"; value: model.state }
                Binding { target: mStageCard; property: "cardType"; value: model.type }
                onReleased: {
                    if (!stagePlaceMouseArea.drag.active)
                        return;
                    if (root.stageDropTarget !== undefined) {
                        root.stageDropTarget.mTooltipsDisabled = true;
                        stage.switchPositions(stagePlace.mIndex, root.stageDropTarget.mIndex);
                        return;
                    }
                    mStageCard.startDropAnim(stagePlace.x, stagePlace.y);
                }

                onEntered: {
                    if (mStageCard === null || drag.active || mTooltipsDisabled)
                        return;

                    let cb = function(status) {
                        if (status !== Component.Ready)
                            return;

                        if (!containsMouse || mStageCard === null) {
                            this.object.destroy();
                            return;
                        }
                        if (mCardInfo !== null)
                            mCardInfo.destroy();
                        mCardInfo = this.object;
                        mCardInfo.x = stagePlace.x + stageRect.width;
                        mCardInfo.y = stagePlace.y;
                        if (model.state === "Rested") {
                            mCardInfo.x += (root.cardHeight - root.cardWidth) / 2;
                            mCardInfo.y += (root.cardHeight - root.cardWidth) / 2;
                        }
                        mCardInfo.mModel = stage.mModel.textModel(model.index);
                        mCardInfo.visible = true;
                    }
                    if (stage.mModel.textModel(index).rowCount() > 0)
                        ObjectCreator.createAsync("CardTextFrame", root, cb);
                }

                onExited: destroyCardInfo()

                onClicked: {
                    if (stage.state === "attack" && model.glow) {
                        destroyCardInfo();
                        stage.declareAttack(mIndex, mouse.button === Qt.LeftButton ? false : true);
                    } else if (stage.state === "encore" && model.glow) {
                        destroyCardInfo();
                        stage.encoreCharacter(mIndex);
                    } else if (model.glow) {
                        if (model.selected && !gGame().getPlayer().hasActivatedAbilities())
                            return;
                        model.selected = !model.selected;
                        gGame.getPlayer().chooseCardOrPosition(model.index, "stage", stage.opponent);
                    } else if (!opponent && mStageCard !== null) {
                        gGame.getPlayer().playActAbility(mIndex);
                    }
                }
            }

            MouseArea {
                id: markerArea

                anchors {
                    top: parent.top
                    left: parent.left
                    topMargin: marker.height * 0.05
                    leftMargin: -marker.width * 0.1
                }
                width: parent.width
                height: parent.height
                visible: mStageCard !== null && model.topMarker
                hoverEnabled: mStageCard !== null

                states: State {
                    name: "hovered"; when: markerArea.containsMouse
                    PropertyChanges { target: markerArea; anchors.leftMargin: -marker.width * 0.2 }
                }

                Behavior on anchors.leftMargin {
                    NumberAnimation { duration: 200 }
                }

                Card {
                    id: marker

                    anchors.fill: parent
                    mSource: model.topMarker
                }

                onClicked: {
                    gGame.getPlayer(stage.opponent).createMarkerView(mIndex);
                }
            }
        }
    }

    // when you place a card and a tooltip appears immediately (annoying)
    SequentialAnimation {
        id: tooltipTimeout
        PauseAnimation { duration: 100 }
        PropertyAction { target: stagePlace; property: "mTooltipsDisabled"; value: false }
    }

    SequentialAnimation {
        id: swapCardsAnim
        property int toIndex
        ScriptAction { script: gGame.startAction(); }
        ParallelAnimation {
            NumberAnimation { id: swapTargetX; target: mStageCard; property: "x"; duration: 150; }
            NumberAnimation { id: swapTargetY; target: mStageCard; property: "y"; duration: 150; }
        }
        ScriptAction { script: finishSwapMove(swapCardsAnim.toIndex); }
    }

    function startSwappingCards(toIndex, x, y) {
        swapCardsAnim.toIndex = toIndex;
        swapTargetX.to = stage.mPositions[toIndex].x;
        swapTargetY.to = stage.mPositions[toIndex].y;
        swapCardsAnim.start();
    }

    function finishSwapMove(toIndex) {
        let code = mStageCard.mSource;
        swapCards(toIndex);
        if (stage.mStagePlaces[toIndex].mStageCard !== null) {
            createStageCardWithAnim(stage.mStagePlaces[toIndex].mStageCard.mSource, stage.mPositions[toIndex].x, stage.mPositions[toIndex].y);
            stage.mStagePlaces[toIndex].createStageCard(code);
            gGame.pause(200);
        } else {
            mStageCard.destroy();
            mStageRect.color = "#30FFFFFF";
            stage.mStagePlaces[toIndex].createStageCard(code);
            gGame.actionComplete();
        }
    }

    function getX() { return mStageCard === null ? stagePlace.x : mStageCard.x; }
    function getY() { return mStageCard === null ? stagePlace.y : mStageCard.y; }

    function destroyCardInfo() {
        if (mCardInfo !== null) {
            mCardInfo.destroy();
            mCardInfo = null;
        }
    }

    function powerChangeAnim() {
        if (mStageCard === null)
            return;
        mStageCard.powerChangeAnim();
    }

    function soulChangeAnim() {
        if (mStageCard === null)
            return;
        mStageCard.soulChangeAnim();
    }

    function levelChangeAnim() {
        if (mStageCard === null)
            return;
        mStageCard.levelChangeAnim();
    }

    function createStageCardWithAnim(code, _x, _y) {
        createCardInternal(code, _x, _y);
        mStageCard.startAnimation(stagePlace.x, stagePlace.y);
    }

    function createStageCard(code) {
        createCardInternal(code, stagePlace.x, stagePlace.y);
    }

    function createCardInternal(code, _x, _y) {
        if (mStageCard !== null) {
            mStageCard.dragActive = false;
            mStageCard.destroy();
        }
        let comp = Qt.createComponent("StageCard.qml");
        let rot = stage.opponent ? 180 : 0;
        mStageCard = comp.createObject(gGame, { x: _x, y: _y, cardRotation: rot});
        mStageCard.mSource = code;
        mStageCard.index = mIndex;
        stagePlace.mTooltipsDisabled = true;
        tooltipTimeout.start();
        mStageRect.color = "#00FFFFFF";
    }

    function setCard(cardId, code) {
        createStageCard(code);
        stage.mModel.setCard(mIndex, cardId, code);
    }

    function removeCard(pos) {
        destroyCardInfo();
        mStageRect.color = "#30FFFFFF";
        mStageCard.destroy();
        stage.mModel.clearCard(pos);
    }

    function swapCards(from) { stage.mModel.swapCards(from, mIndex); }
    function sendToWr() { stage.sendToWr(mIndex); }
}
