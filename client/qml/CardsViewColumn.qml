import QtQuick 2.12
import QtQml 2.12
import QtQml.Models 2.12

import wsamateur 1.0

import "objectCreation.js" as ObjectCreator

ListView {
    id: thisListView

    property CardModel mModel
    property var num
    property var predicate
    property real effectiveHeight: contentHeight + topMargin + bottomMargin
    property CardTextFrame cardTextFrame: null

    height: Math.min(effectiveHeight, cardsView.mColumnMaxHeight)
    width: contentWidth + leftMargin + rightMargin
    spacing: -root.cardHeight * 0.7
    clip: true
    contentWidth: root.cardWidth
    leftMargin: root.cardWidth * 0.1
    rightMargin: root.cardWidth * 0.1
    topMargin: root.cardHeight * 0.1
    bottomMargin: root.cardHeight * 0.1
    visible: count
    interactive: effectiveHeight > cardsView.mColumnMaxHeight

    model: DelegateModel {
        id: listDelegate

        model: mModel

        groups: [
            DelegateModelGroup {
                includeByDefault: false
                name: "visibleGroup"
            }
        ]
        filterOnGroup: "visibleGroup"

        items.onChanged: {
            let cur = 0;
            while (cur < items.count) {
                let item = items.get(cur);
                if (predicate(item.model))
                    item.groups = "visibleGroup";
                else
                    cur++;
            }
        }

        delegate: Component {
            MouseArea {
                id: cardDelegate

                width: root.cardWidth
                height: root.cardHeight
                hoverEnabled: true

                onEntered: {
                    cardImgDelegate.state = "hovered";
                }
                onExited: {
                    cardImgDelegate.state = "";
                }
                onPressed: {
                    cardImgDelegate.state = "";
                }
                onClicked: {
                    if (!model.glow)
                        return;
                    model.selected = !model.selected;
                    gGame.getPlayer(false).chooseCard(model.index, cardsView.mZoneName, cardsView.mOpponent);
                }

                Card {
                    id: cardImgDelegate
                    mSource: model.code
                    anchors.centerIn: cardDelegate
                    Component.onDestruction: destroyTextFrame(cardImgDelegate)

                    states: State {
                        name: "hovered"
                        PropertyChanges {
                            target: cardImgDelegate
                            scale: 1.1
                            z: 100
                        }
                        PropertyChanges {
                            target: thisListView
                            z: 100
                        }
                        ParentChange {
                            target: cardImgDelegate
                            parent: thisListView
                        }
                        StateChangeScript {
                            name: "textFrame"
                            script: createTextFrame(cardImgDelegate, model.index);
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
    }

    function createTextFrame(frameParent, index) {
        const cb = function(status) {
            if (status !== Component.Ready)
                return;

            if (frameParent.state !== "hovered") {
                this.object.destroy();
                return;
            }
            if (thisListView.cardTextFrame !== null)
                thisListView.cardTextFrame.destroy();
            let textFrame = this.object;

            let listViewMappedPoint = root.mapFromItem(thisListView, thisListView.x, thisListView.y);
            let cardMappedPoint = root.mapFromItem(frameParent, frameParent.x, frameParent.y);
            if (!cardsView.mOpponent)
                textFrame.x = cardMappedPoint.x - textFrame.width - root.cardWidth * 0.1 - 1;
            else
                textFrame.x = cardMappedPoint.x + root.cardWidth - 1;
            let scaleYOffset = root.cardHeight * 0.1 * 0.5;
            textFrame.y = listViewMappedPoint.y + frameParent.y - scaleYOffset;

            if (textFrame.y + textFrame.height > root.height)
                textFrame.y -= textFrame.y + textFrame.height - root.height;
            textFrame.mModel = thisListView.mModel.textModel(index);
            textFrame.visible = true;
            thisListView.cardTextFrame = textFrame;
        }

        if (thisListView.mModel.textModel(index).rowCount() > 0)
            // ListView is clipping, so root parent here
            ObjectCreator.createAsync("CardTextFrame", root, cb);
    }

    function destroyTextFrame(frameParent) {
        if (thisListView.cardTextFrame !== null) {
            thisListView.cardTextFrame.destroy();
            thisListView.cardTextFrame = null;
        }
    }
}
