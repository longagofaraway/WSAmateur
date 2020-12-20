import QtQuick 2.12
import QtQml.Models 2.12

ListView {
    id: stagePlace

    property int mIndex
    property StageCard mStageCard: null
    property bool mTooltipsDisabled: false

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

            property CardInfoFrame cardInfo: null

            width: root.cardWidth
            height: root.cardHeight
            color: mStageCard === null ? "#30FFFFFF" : "#00FFFFFF"

            DropArea {
                id: dropStage
                width: root.cardWidth * 2
                height: stagePlace.mIndex < 3 ? root.cardHeight * 3/2 : root.cardHeight
                x: -(width - root.cardWidth) / 2
                y: stagePlace.mIndex < 3 ? 0 : root.cardHeight / 2

                onEntered: {
                    if (stage.opponent)
                        return;

                    stageRect.color = "#80FFFFFF";
                    root.stageDropTarget = stagePlace;
                }

                onExited: {
                    if (stage.opponent)
                        return;

                    root.stageDropTarget = undefined;
                    stageRect.color = "#30FFFFFF";
                }
            }

            MouseArea {
                id: stagePlaceMouseArea

                anchors.fill: parent
                hoverEnabled: true
                drag.target: (stage.opponent || !stage.mDragEnabled) ? undefined : mStageCard
                drag.onActiveChanged: {
                    if (active)
                        destroyCardInfo();
                }

                Binding { target: mStageCard; property: "dragActive"; value: stagePlaceMouseArea.drag.active }
                onReleased: {
                    if (!stagePlaceMouseArea.drag.active)
                        return;
                    if (root.stageDropTarget !== undefined) {
                        let swappingCard = model.code;
                        let dropTarget = root.stageDropTarget;
                        dropTarget.swapCards(stagePlace.mIndex);
                        if (dropTarget.mStageCard === null)
                            mStageCard.destroy();
                        else
                            createStageCardWithAnim(model.code, dropTarget.x, dropTarget.y);
                        dropTarget.createStageCard(swappingCard);
                        stage.switchPositions(stagePlace.mIndex, dropTarget.mIndex);
                        return;
                    }
                    mStageCard.x = stagePlace.x;
                    mStageCard.y = stagePlace.y;
                }

                onEntered: {
                    if (mStageCard === null || drag.active || mTooltipsDisabled)
                        return;
                    let comp = Qt.createComponent("CardInfoFrame.qml");
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
        stagePlace.mTooltipsDisabled = true;
        tooltipTimeout.start();
    }

    function setCard(code) {
        createStageCard(code);
        stage.mModel.setCard(mIndex, code);
    }

    function swapCards(from) {
        stage.mModel.swapCards(from, mIndex);
    }
}
