import QtQuick 2.12
import QtQml.Models 2.12
import QtGraphicalEffects 1.12

import wsamateur 1.0

ListView {
    id: stagePlace

    property int mIndex
    property StageCard mStageCard: null
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
            let cur = 0;
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

            property CardTextFrame cardInfo: null

            width: root.cardWidth
            height: root.cardHeight
            color: "#30FFFFFF"

            Component.onCompleted: { mStageRect = stageRect; }

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
                        || drag.source.cardType === undefined)
                        return;
                    if (drag.source.index === mIndex)
                        return;

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
                rotation: (model.state === "Rested") ? 90 : 0
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                drag.target: (stage.opponent || !stage.mDragEnabled) ? undefined : mStageCard
                drag.onActiveChanged: {
                    if (active)
                        destroyCardInfo();
                }

                Binding { target: mStageCard; property: "dragActive"; value: stagePlaceMouseArea.drag.active }
                Binding { target: mStageCard; property: "glow"; value: model.glow }
                Binding { target: mStageCard; property: "selected"; value: model.selected }
                Binding { target: mStageCard; property: "power"; value: model.power }
                Binding { target: mStageCard; property: "soul"; value: model.soul }
                Binding { target: mStageCard; property: "cardState"; value: model.state }
                Binding { target: mStageCard; property: "cardType"; value: model.type }
                onReleased: {
                    if (!stagePlaceMouseArea.drag.active)
                        return;
                    if (root.stageDropTarget !== undefined) {
                        let swappingCard = model.code;
                        let dropTarget = root.stageDropTarget;
                        dropTarget.swapCards(stagePlace.mIndex);
                        if (dropTarget.mStageCard === null) {
                            stageRect.color = "#30FFFFFF";
                            mStageCard.destroy();
                        } else {
                            createStageCardWithAnim(model.code, dropTarget.x, dropTarget.y);
                        }
                        dropTarget.createStageCard(swappingCard);
                        stage.switchPositions(stagePlace.mIndex, dropTarget.mIndex);
                        return;
                    }
                    mStageCard.startDropAnim(stagePlace.x, stagePlace.y);
                }

                onEntered: {
                    if (mStageCard === null || drag.active || mTooltipsDisabled)
                        return;
                    let comp = Qt.createComponent("CardTextFrame.qml");
                    let incubator = comp.incubateObject(root, { visible: false, z: 100 }, Qt.Asynchronous);
                    let createdCallback = function(status) {
                        if (status === Component.Ready) {
                            if (!containsMouse) {
                                incubator.object.destroy();
                                return;
                            }
                            if (cardInfo !== null)
                                cardInfo.destroy();
                            cardInfo = incubator.object;
                            cardInfo.x = stagePlace.x + stageRect.width;
                            cardInfo.y = stagePlace.y;
                            if (model.state === "Rested") {
                                cardInfo.x += (root.cardHeight - root.cardWidth) / 2;
                                cardInfo.y += (root.cardHeight - root.cardWidth) / 2;
                            }
                            cardInfo.visible = true;
                        }
                    }
                    if (incubator.status !== Component.Ready) {
                        incubator.onStatusChanged = createdCallback;
                    } else {
                        createdCallback(Component.Ready);
                    }
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
                        model.selected = !model.selected;
                        gGame.getPlayer().chooseCard(model.index, "stage", stage.opponent);
                    }
                }

                function destroyCardInfo() {
                    if (cardInfo !== null) {
                        cardInfo.destroy();
                        cardInfo = null;
                    }
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
        mStageCard = comp.createObject(gGame, { x: _x, y: _y });
        mStageCard.mSource = code;
        mStageCard.index = mIndex;
        stagePlace.mTooltipsDisabled = true;
        tooltipTimeout.start();
        mStageRect.color = "#00FFFFFF";
    }

    function setCard(code) {
        createStageCard(code);
        stage.mModel.setCard(mIndex, code);
    }
    function removeCard(pos) {
        mStageRect.color = "#30FFFFFF";
        mStageCard.destroy();
        stage.mModel.clearCard(pos);
    }

    function swapCards(from) { stage.mModel.swapCards(from, mIndex); }
    function sendToWr() { stage.sendToWr(mIndex); }
}
