import QtQuick 2.12
import QtQml 2.12
import QtQml.Models 2.12

import wsamateur 1.0

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
                    gGame.getPlayer(false).chooseCard(model.index, "wr");
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
                            script: createTextFrame(cardImgDelegate);
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

    function createTextFrame(cardObject) {
        let comp = Qt.createComponent("CardTextFrame.qml");
        let incubator = comp.incubateObject(root, { visible: false }, Qt.Asynchronous);
        let createdCallback = function(status) {
            if (status === Component.Ready) {
                if (cardObject.state !== "hovered") {
                    incubator.object.destroy();
                    return;
                }
                if (thisListView.cardTextFrame !== null)
                    thisListView.cardTextFrame.destroy();
                let textFrame = incubator.object;

                let listViewMappedPoint = root.mapFromItem(thisListView, thisListView.x, thisListView.y);
                let cardMappedPoint = root.mapFromItem(cardObject, cardObject.x, cardObject.y);
                textFrame.x = cardMappedPoint.x - textFrame.width;
                let scaleOffset = root.cardHeight * 0.1 * 0.5;
                textFrame.y = listViewMappedPoint.y + cardObject.y - scaleOffset;

                if (textFrame.y + textFrame.height > root.height)
                    textFrame.y -= textFrame.y + textFrame.height - root.height;
                textFrame.visible = true;
                thisListView.cardTextFrame = textFrame;
            }
        }
        if (incubator.status !== Component.Ready) {
            incubator.onStatusChanged = createdCallback;
        } else {
            createdCallback(Component.Ready);
        }
    }

    function destroyTextFrame(frameParent) {
        if (thisListView.cardTextFrame !== null) {
            thisListView.cardTextFrame.destroy();
            thisListView.cardTextFrame = null;
        }
    }
}
